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

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
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
      _in(),
      _in_off(0),
      _peerClosedInput(false),
      _pendingResponses(),
      _req_buffer()
{
}

bool Connection::hasPendingResponses() const
{
    return !_pendingResponses.empty();
}

Connection::Connection(const Connection &other)
    : _fd(other._fd),
      _srv(other._srv),
      _status(other._status),
      _req(other._req),
      _pendingResponses(other._pendingResponses)
{}

Connection::Connection(int fd, TCPServer* srv)
    : _fd(fd),
      _srv(srv),
      _status(READING_HEADERS),
      _req(NULL),
      _in(),
      _in_off(0),
      _peerClosedInput(false),
      _pendingResponses(),
      _req_buffer()
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
        {
            // Peer closed its write-side (FIN). We might still have a full request in _in.
            _peerClosedInput = true;
            return OK;
        }

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
    if (_pendingResponses.empty())
        return OK;

    PendingResponse& item = _pendingResponses.front();
    if (!item.res)
        return ERROR;

    HttpResponse* cur = item.res;
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

            const bool close_after = item.closeAfter;

            delete cur;
            _pendingResponses.pop_front();

            // If the request said "Connection: close", close *after* we sent its response.
            if (close_after)
                return CLOSED;

            if (!_pendingResponses.empty())
                return WANT_WRITE;

            _status = READING_HEADERS;

            Connection::e_result pr = _processInput(); // try to parse / enqueue immediately so we don't wait for another EPOLLIN.
            if (pr == WANT_WRITE)
                return WANT_WRITE;
            return OK;
        }
        return WANT_WRITE;
    }

    if (w == 0)
        return CLOSED;

    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        return WANT_WRITE;

    if (errno == EPIPE || errno == ECONNRESET)
        return CLOSED;

    return ERROR;
}

void Connection::_consumeInputBytes(size_t nbytes)
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

    for (size_t i = 0; i < conn.size(); i++)  // normalize to lowercase
        conn[i] = static_cast<char>(std::tolower(conn[i]));

    if (proto.find("HTTP/1.1") == 0)
        return conn != "close";
    return conn == "keep-alive";
}

/**
 * if we have headers (and body if needed),
 * consume bytes and enqueue a response”.
 * @return
 */
bool Connection::_tryExtractOneRequest()
{
    size_t hdr_end = _in.find(CRLF CRLF, _in_off);
    if (hdr_end == std::string::npos)
        return false;

    const size_t header_bytes = (hdr_end - _in_off) + 4;
    const std::string header_block = _in.substr(_in_off, header_bytes);

    std::cout << FT_MAGENTA << "Request ready on fd: " << _fd << std::endl;
    std::cout << FT_GREEN << header_block << FT_RESET << std::endl;

	HttpRequest* req = new HttpRequest();
	try {
		req->parseRequest(header_block);
	} catch (HttpRequest::GenericException&) {
        _srv->requests_failed++;
        _in.erase();
        delete _req;
        _req = NULL;
        return false; // err
	}

    size_t content_length = 0;
    if (req->headers.count("content-length"))
        content_length = static_cast<size_t>(std::atoi(req->headers["content-length"].c_str()));
    req->content_length = content_length;

	const size_t need_total = header_bytes + content_length;
	if (_in.size() - _in_off < need_total)
	{
		delete req;
		return false; // not enough body yet
	}

    // TODO: reuse extract body function
	if (req->content_length > 0)
	{
		req->body.resize(content_length);
		std::memcpy(req->body.data(), _in.data() + _in_off + header_bytes, content_length);
	}

    _consumeInputBytes(need_total);

    delete _req;
    _req = req;

    HttpResponse* res = _prepareResponse();
    res->buildHttpResponse();
    _req->printBody();

    const PendingResponse &presp = (PendingResponse) {
        .res = res,
        .closeAfter = !_shouldKeepAlive(*_req)};
    _pendingResponses.push_back(presp);

    _status = READY_TO_WRITE;
    return true;
}

/**
 * Repeatedly try to parse as many complete requests as are already
 * buffered and append responses to the queue.
 * @see HTTP/1.1 pipelining.
 * @return
 */
Connection::e_result Connection::_processInput()
{
    // If we're currently writing, we still can parse and enqueue more
    // pipelined requests, but we should not drop back to EPOLLIN-only
    // if there's data to send.
    bool queued_any = false;

    while (true)
    {
        const size_t before = _pendingResponses.size();
        if (!_tryExtractOneRequest())
            break;
        if (_pendingResponses.size() > before)
            queued_any = true;
    }

    if (!_pendingResponses.empty()) // switchong to EPOLLOUT
        return WANT_WRITE;

    return queued_any ? WANT_WRITE : OK;
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

    // If peer already closed input and we produced nothing to write, close now.
    if (_peerClosedInput && _pendingResponses.empty())
        return CLOSED;

    return OK;
}

Connection::e_result Connection::onWritable()
{
    if (_status != READY_TO_WRITE)
        return OK;
    return _sendToClient();
}

int	cgi_handle(HttpRequest& req, HttpResponse& res, CGI *cgi)
{
	int in_pipe[2];
	int out_pipe[2];

	pipe(in_pipe);
	pipe(out_pipe);


    pid_t child_pid = fork();
    if (child_pid == 0) // child
	{
        dup2(out_pipe[1], STDOUT_FILENO); // stdout -> pipe
		close(out_pipe[0]);
		close(out_pipe[1]);

        dup2(in_pipe[0], STDIN_FILENO); // pipe -> stdin
		close(in_pipe[1]);
		close(in_pipe[0]);

		// build ENVP
		
		std::string script = apply_location(req.path, res.location);
		// build ARGV
		

		const char *argv[3] = {
			"/usr/bin/python3",
			script.c_str(),
			NULL,
		};
		const char *envp[3] = {
			"FUCK=me",
			"TWAT=you",
			NULL,
		};
		execve(argv[0], (char *const *)argv, (char *const *)envp);
	}
	else if (child_pid < 0)
	{
		return (1);
	}

	close(out_pipe[1]);
	close(in_pipe[0]);

	write(in_pipe[1], "", 0);
	close(in_pipe[1]);

	FILE*	fp = fdopen(out_pipe[0], "r");
	char	*line = NULL;
	size_t	n = 0;
	ssize_t	nread = 0;
	while ((nread = getline(&line, &n, fp)) != -1)
	{
		res.body.append(line);
	}
	fclose(fp);
	free(line);
	wait(NULL);
	return 0;
	(void)cgi;
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
		if (cgi_handle(*_req, *res, cgi))
		{
			res->set_response_code(HttpResponse::SC_500);
			_handleErrorResponse(res);
			return res;
		}
		// res->body = "{\"return\": \"OK\"}";
		res->headers["content-type"] = "text/plain";
		res->headers["content-length"] = ::itoa(res->body.length());
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
    while (!_pendingResponses.empty())
    {
        delete _pendingResponses.front().res;
        _pendingResponses.pop_front();
    }

    delete _req;
    _req = NULL;

    _in.clear();
    _in_off = 0;

    _peerClosedInput = false;
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
