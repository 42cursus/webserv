/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 04:08:12 by abelov            #+#    #+#             */
/*   Updated: 2026/01/29 06:40:43 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "TCPServer.hpp"
#include "Prefix_suffix.hpp"
#include "Location.hpp"
#include "webserv.hpp"

/*
** -------------------------------- STATIC VARS -------------------------------
*/

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

Connection::Connection()
    : _fd(-1),
      _srv(NULL),
      _status(REQ_HEADERS),
      _req(NULL),
      _res(NULL),
      _req_buffer()
{
}

Connection::Connection(const Connection &other)
    : _fd(other._fd),
      _srv(other._srv),
      _status(other._status),
      _rawRequest(other._rawRequest),
      _req(other._req),
      _res(other._res) {}


/*
** ------------------------------- DESTRUCTORS --------------------------------
*/

Connection::~Connection()
{
    if (_fd != -1)
    {
        ::close(_fd);
        _fd = -1;
    }
    reset();
}

/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

Connection::Result Connection::onReadable()
{
    ssize_t nread = ::read(_fd, _req_buffer, 4096);
    if (nread <= 0)
        return CLOSED;

    _req_buffer[nread] = '\0';

    switch (_status) {
        case REQ_HEADERS: {
            size_t old_size = _rawRequest.size();
            _rawRequest += _req_buffer;

            size_t clcr_pos = _rawRequest.find(CRLF CRLF);
            if (clcr_pos == std::string::npos)
                return OK;

            _req = new HttpRequest();
            try {
                _req->parseRequest(_rawRequest);
            } catch (HttpRequest::GenericException&) {
                _srv->requests_failed++;
                _rawRequest.erase();
                delete _req;
                _req = NULL;
                return ERROR;
            }

            _req->content_length = std::atoi(_req->headers["content-length"].c_str());
            if (_req->content_length > 0)
            {
                _status = REQ_BODY;
                size_t body_size = extract_body(static_cast<size_t>(nread), old_size, clcr_pos);
                if (body_size < _req->content_length)
                    return OK;
            }
            break;
        }
        case REQ_BODY: {
            size_t old_size = _req->body.size();
            _req->body.resize(old_size + static_cast<size_t>(nread));
            std::memcpy(_req->body.data() + old_size, _req_buffer, static_cast<size_t>(nread));
            if (_req->body.size() < _req->content_length)
                return OK;
            break;
        }
        default:
            break;
    }

    _res = prepareResponse();
    _res->buildHttpResponse();
    _status = REQ_RESPONSE_READY;
    return WANT_WRITE;
}

Connection::Result Connection::onWritable()
{
    if (!_res)
        return ERROR;

    std::string& response = _res->response;
    size_t msg_size = response.length() - _res->start;
    msg_size = std::min(msg_size, response.length() - _res->start);

    ssize_t w = ::write(_fd, &response.c_str()[_res->start], msg_size);
    if (w <= 0)
        return OK;

    _res->start += static_cast<size_t>(w);
    if (_res->start >= response.length())
    {
        _srv->requests_handled++;
        _rawRequest.erase();
        reset();
        return OK;
    }
    return WANT_WRITE;
}

HttpResponse* Connection::prepareResponse() const
{
    HttpResponse* res = new HttpResponse();
    Location* location = loc_trie_search(_srv->getCfg().http.server.loc_trie, _req->path);

    res->statuscode = "200";
    res->statusmsg = "OK";
    res->headers["Server"] = "Webserv/0.69";

    if (!location)
    {
        res->statuscode = "404";
        res->statusmsg = "Not Found";
        handle_error_response(res);
        return res;
    }

    if (std::find(location->_methods.begin(), location->_methods.end(), _req->method) == location->_methods.end())
    {
        res->statuscode = "405";
        res->statusmsg = "Method Not Allowed";
        handle_error_response(res);
        return res;
    }

    CGI* cgi = cgi_trie_search(location->cgi_trie, _req->path);
    if (cgi != NULL)
    {
        res->statuscode = "500";
        res->statusmsg = "Internal Server Error";
        handle_error_response(res);
        return res;
    }

    try {
        if (_req->method == "GET")
        {
            res->body = _req->getHtmlResponse(location, *res);
            if (_req->headers["range"].empty())
                res->headers["accept-ranges"] = "bytes";
            else if (_req->headers["range"].find("bytes") == 0)
            {
                parse_range(*res);
                res->statuscode = "206";
                res->statusmsg = "Partial Content";
            }
            res->headers["content-length"] = ::itoa(res->body.length());
        }
        else if (_req->method == "PUT")
        {
            std::string rel_path = _req->path.substr(location->_path.length(), _req->path.length());
            if (rel_path.empty())
                rel_path = "default";
            std::string path = location->_root + rel_path;

            if (access(path.c_str(), F_OK) == 0)
            {
                res->statuscode = "204";
                res->statusmsg = "No Content";
            }
            else
            {
                res->statuscode = "201";
                res->statusmsg = "Created";
            }
            res->headers["content-length"] = "0";
            int fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
            if (fd >= 0)
            {
                if (!_req->body.empty())
                    ::write(fd, _req->body.data(), _req->body.size());
                close(fd);
            }
        }
        else if (_req->method == "DELETE")
        {
            // TODO
        }
    } catch (std::exception&) {
        handle_error_response(res);
    }

    return res;
}

void Connection::handle_error_response(HttpResponse* res) const
{
    std::string path = _srv->getCfg().http.server.error_pages.at(res->statuscode);
    if (!path.empty() && path[0] != '.')
    {
        Location* location = loc_trie_search(_srv->getCfg().http.server.loc_trie, path);
        path = apply_location(path, location);
    }
    res->headers["content-type"] = _req->getMimeType(path);
    res->body = res->readHtmlFile(path);
    res->headers["content-length"] = ::itoa(res->body.length());
}

void Connection::parse_range(HttpResponse& res) const
{
    std::string rangestr = _req->headers["range"];
    size_t i = rangestr.find('=');
    char* endptr;
    size_t start = std::strtol(&rangestr.c_str()[i + 1], &endptr, 10);

    if (*endptr != '-')
        return;
    endptr++;
    size_t end = std::strtol(endptr, &endptr, 10);
    if (end == 0)
        end = res.body.length() - 1;

    res.headers["content-range"] = "bytes " + ::itoa(start) + "-" + ::itoa(end) + "/" + ::itoa(res.body.length());
    if (end < res.body.length() - 1)
        res.body.erase(end + 1);
    res.body.erase(0, start);
}

size_t Connection::extract_body(size_t nread, size_t old_size, size_t clcr_pos) const
{
    const size_t body_start = clcr_pos + 4 - old_size;
    const size_t body_size = nread - body_start;
    const char*  src = &_req_buffer[0] + body_start;

    _req->body.resize(body_size);
    std::memcpy(_req->body.data(), src, body_size);
    return body_size;
}

void Connection::clearRequest()
{
    if (!_rawRequest.empty())
        _rawRequest.erase();
}

void Connection::closeSocketFd()
{
    int fd = _fd;
    if (fd != -1)
        ::close(fd);
    _fd = -1;
}

void Connection::reset()
{
    delete _res;
    delete _req;
    _res = NULL;
    _req = NULL;
    _status = REQ_HEADERS;
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

void Connection::setSrv(TCPServer* srv) { _srv = srv; }
void Connection::setFd(int fd)
{
    _fd = fd;
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

int Connection::getFd() const { return _fd; }
Connection::e_status Connection::getStatus() const { return _status; }

/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/
