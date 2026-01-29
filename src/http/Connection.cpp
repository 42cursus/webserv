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
#include <cstdio>
#include <unistd.h>

#include "Location.hpp"
#include "Prefix_suffix.hpp"
#include "TCPServer.hpp"
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
      _status(READING_HEADERS),
      _req(NULL),
      _res(NULL),
      _in_off(0),
      _expected_body(0),
      _keep_alive_for_current(true),
      _req_buffer()
{
}

Connection::Connection(const Connection &other)
    : _fd(other._fd),
      _srv(other._srv),
      _status(other._status),
      _req(other._req),
      _res(other._res) {}

Connection::Connection(int fd, TCPServer* srv)
    : _fd(fd),
      _srv(srv),
      _status(READING_HEADERS),
      _header_end(std::string::npos),
      _req(),
      _res(),
      _write_off(0)
{
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

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

Connection::e_result Connection::_recvFromClient()
{
    while (true)
    {
        ssize_t nread = ::read(_fd, _req_buffer, REQUEST_BUF_SIZE);
        if (nread > 0)
        {
            _req_buffer[nread] = '\0';
            _in.append(_req_buffer, static_cast<size_t>(nread));
            continue; // drain the kernel buffer
        }
        if (nread == 0)
            return CLOSED;

        if (_status == READING_BODY) {
            size_t old_size = _req->body.size();
            _req->body.resize(old_size + nread);
            std::memcpy(_req->body.data() + old_size, _req_buffer, nread);
            _req->printBody();
        }

        if (errno == EINTR)
            continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return OK;

        return ERROR;
    }
}

void logServingFile(const std::string& path, const std::string& mimetype) {
    std::cout << "Serving file: " << path << " with MIME type: " << mimetype << std::endl;
}

Connection::e_result Connection::_sendToClient()
{
    if (_outq.empty())
        return OK;

    HttpResponse* cur = _outq.front();
    if (!cur)
        return ERROR;

    std::string& response = cur->response;
    if (cur->start >= response.size())
        return OK;
    if (cur->start == 0)
    {
        std::string& type = cur->headers["content-type"];
        if (!cur->body.empty())
            logServingFile(cur->filename, type);
        if (!type.empty() && type.substr(0, type.find_first_of("/")) == "text")
            std::cout << FT_BLUE << response << FT_RESET << std::endl;
        else
            std::cout << FT_BLUE << response.substr(0, response.find(CRLF CRLF)) << "\n<Binary file>" << FT_RESET << std::endl;
    }
    // size_t	msg_size = RESPONSE_MSG_SIZE;
    size_t	remaining = response.length() - cur->start;
    // resize_socket_buffer(_conn_fd, msg_size);
    remaining = std::min(remaining, response.length() - cur->start);
    ssize_t w = ::write(_fd, response.data() + cur->start, remaining);

    if (w > 0)
    {
        cur->start += static_cast<size_t>(w);
        if (cur->start >= response.size())
        {
            _srv->requests_handled++;

            bool keep = _keep_alive_for_current;
            delete cur;
            _outq.pop_front();

            if (_outq.empty())
            {
                if (!keep)
                    return CLOSED;

                _status = READING_HEADERS;

                Connection::e_result pr = _processInput(); // try to parse / enqueue immediately so we don't wait for another EPOLLIN.
                if (pr == WANT_WRITE)
                    return WANT_WRITE;
                return OK;
            }
            return WANT_WRITE;
        }
        return WANT_WRITE;
    }

    if (w == 0)
        return CLOSED;

    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        return WANT_WRITE;

    return ERROR;
}

void Connection::_consume(size_t nbytes)
{
    _in_off += nbytes;

    if (_in_off > 65536 || (_in_off > 0 && _in_off * 2 > _in.size()))
    {
        _in.erase(0, _in_off);
        _in_off = 0;
    }
}

void Connection::_resetCurrentRequest()
{
    delete _req;
    _req = NULL;
    _expected_body = 0;
    _keep_alive_for_current = true;
    _status = READING_HEADERS;
}

/**
 * - HTTP/1.1 => keep-alive by default unless "connection: close"
 * - HTTP/1.0 => close by default unless "connection: keep-alive"
 * @param req
 * @return
 */
bool Connection::_shouldKeepAlive(const HttpRequest& req) const
{
    std::string proto = req.protocol;
    std::string conn = req.headers.count("connection") ? req.headers.find("connection")->second : "";

    // normalize to lowercase
    for (size_t i = 0; i < conn.size(); i++)
        conn[i] = static_cast<char>(std::tolower(conn[i]));

    if (proto.find("HTTP/1.1") == 0)
        return conn != "close";
    return conn == "keep-alive";
}

bool Connection::_tryExtractOneRequest()
{
    size_t hdr_end = _in.find(CRLF CRLF, _in_off);
    if (hdr_end == std::string::npos)
        return false;

    const size_t body_start = (hdr_end - _in_off) + 4;
    const std::string header_block = _in.substr(_in_off, body_start);

    std::cout << FT_MAGENTA << "Request ready on fd: " << _fd << std::endl;
    std::cout << FT_GREEN << header_block << FT_RESET << std::endl;

	HttpRequest* req = new HttpRequest();
	try {
		req->parseRequest(header_block);
	} catch (HttpRequest::GenericException&) {
        _srv->requests_failed++;
        _raw.erase();
        delete _req;
        _req = NULL;
        return false; // err
	}

    size_t content_length = 0;
    if (req->headers.count("content-length"))
        content_length = static_cast<size_t>(std::atoi(req->headers["content-length"].c_str()));
    req->content_length = content_length;

	const size_t need_total = body_start + content_length;
	if (_in.size() - _in_off < need_total)
	{
		delete req;
		return false; // not enough body yet
	}

    // TODO: reuse extract body function
	if (req->content_length > 0)
	{
		req->body.resize(content_length);
		std::memcpy(req->body.data(), _in.data() + _in_off + body_start, content_length);
	}

    _consume(need_total);

    delete _req;
    _req = req;

    _req->printBody();
    _keep_alive_for_current = _shouldKeepAlive(*_req); // FIXME

    HttpResponse* res = _prepareResponse();
    res->buildHttpResponse();
    _outq.push_back(res);

    _status = READY_TO_WRITE;
    return true;
}

Connection::e_result Connection::_processInput()
{
    // If we're currently writing, we still can parse and enqueue more
    // pipelined requests, but we should not drop back to EPOLLIN-only
    // if there's data to send.
    bool enqueued_any = false;

    while (true)
    {
        const size_t before = _outq.size();
        if (!_tryExtractOneRequest())
            break;
        if (_outq.size() > before)
            enqueued_any = true;
    }

    if (!_outq.empty())
        return WANT_WRITE;

    return enqueued_any ? WANT_WRITE : OK;
}

Connection::e_result Connection::onReadable()
{
    e_result rr = _recvFromClient();
    if (rr != OK)
        return rr;

    // parse/enqueue as much as possible
    e_result pr = _processInput();
    if (pr == WANT_WRITE)
        return WANT_WRITE;

    return OK;
}

Connection::e_result Connection::onWritable()
{
    if (_status != READY_TO_WRITE)
        return OK;
    return _sendToClient();
}

HttpResponse* Connection::_prepareResponse() const
{
	HttpResponse*	res = new HttpResponse();
	res->location = loc_trie_search(_srv->getCfg().http.server.loc_trie, _req->path);
	res->filename = _req->path.substr(res->location->_path.length(), _req->path.length());
	std::string		mimetype;

	res->set_response_code(HttpResponse::SC_200);
	res->headers["Server"] = "Webserv/0.69";

	if (!_req->is_method_permitted(res->location))
	{
		res->set_response_code(HttpResponse::SC_405);
		_handleErrorResponse(res);
		return res;
	}

	CGI	*cgi = cgi_trie_search(res->location->cgi_trie, _req->path);
	if (cgi != NULL)
	{
		res->set_response_code(HttpResponse::SC_500);
		res->headers["boop"] = "beep";
		_handleErrorResponse(res);
		return res;
	}

	try {
		switch (_req->get_method()) {
			case (HttpRequest::GET):
				_prepareResponse_get(res);
				break;
			case (HttpRequest::PUT):
				_prepareResponse_put(res);
				break;
			case (HttpRequest::POST):
				_prepareResponse_post(res);
				break;
			case (HttpRequest::DELETE):
				_prepareResponse_delete(res);
				break;
		}
	} catch (std::exception &e)
	{
		_handleErrorResponse(res);
	}

	return (res);
}

void Connection::_prepareResponse_get(HttpResponse *res) const
{
	res->body = _req->getHtmlResponse(*res);

	if (_req->headers["range"].empty())
		res->headers["accept-ranges"] = "bytes";
	else if (_req->headers["range"].find("bytes") == 0)
	{
		_parseRange(*res);
		res->set_response_code(HttpResponse::SC_206);
	}
	res->headers["content-length"] = ::itoa(res->body.length());

}

void Connection::_prepareResponse_put(HttpResponse *res) const
{
	std::string	rel_path = _req->path.substr(res->location->_path.length(), _req->path.length());
	if (rel_path.empty())
		rel_path = "default";
	std::string	path = res->location->_root + rel_path;

	if (access(path.c_str(), F_OK) == 0)
		res->set_response_code(HttpResponse::SC_204);
	else
		res->set_response_code(HttpResponse::SC_201);

	res->headers["content-length"] = "0";
	int	fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
	write(fd, _req->body.data(), _req->body.size());
	close(fd);

}

void Connection::_prepareResponse_post(HttpResponse *res) const
{
	(void)res;
}

void Connection::_prepareResponse_delete(HttpResponse *res) const
{
	std::string path = res->location->_root + res->filename;

	if (access(path.c_str(), F_OK) != 0)
	{
		res->set_response_code(HttpResponse::SC_404);
		_handleErrorResponse(res);
		return ;
	}

	if (access(path.c_str(), W_OK) != 0)
	{
		res->set_response_code(HttpResponse::SC_403);
		_handleErrorResponse(res);
		return ;
	}

	int retval = std::remove(path.c_str());
	if (retval == 0)
	{
		res->set_response_code(HttpResponse::SC_204);
		res->headers["content-length"] = "0";
	}
	else {
		; // handle_error
	}
}


void Connection::_handleErrorResponse(HttpResponse* res) const
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

void Connection::_parseRange(HttpResponse& res) const
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

void Connection::clearRequest()
{
	_in.clear();
	_in_off = 0;
	_resetCurrentRequest();
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
    // drain queued responses
    while (!_outq.empty())
    {
        delete _outq.front();
        _outq.pop_front();
    }

    delete _res;
    delete _req;
    _res = NULL;
    _req = NULL;

    _in.clear();
    _in_off = 0;
    _expected_body = 0;
    _keep_alive_for_current = true;

    _status = READING_HEADERS;
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
