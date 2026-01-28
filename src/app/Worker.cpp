/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:02:20 by abelov            #+#    #+#             */
/*   Updated: 2026/01/21 18:16:04 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"

#include <asm-generic/socket.h>
#include <cassert>

#include "webserv.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Prefix_suffix.hpp"
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>

Worker::Worker()
	: _req_buffer(),
    _req(NULL),
    _res(NULL),
	_conn_fd(-1),
	_request_handled(),
	_addr(),
	_addr_size(),
	_srv(NULL),
	_status(REQ_HEADERS)
{

}

Worker::~Worker()
{

}

/**
 * `::` = "start lookup in the global namespace"
 * C++ normally searches for a name in the following order:
 *	- Local scope
 *	- Class scope
 *	- Namespace scope(s)
 *	- Global scope
 *	- Argument-dependent lookup (ADL)
 *
 *	with `::` it skips all above and directly jumps to the global namespace.
 */
void Worker::acceptConnection()
{
	struct sockaddr *addr = reinterpret_cast<struct sockaddr*>(&_addr); // NOLINT(*-pro-type-reinterpret-cast)
	_conn_fd = /* global namespace */ ::accept(_srv->getSocketFd(), addr, &_addr_size);
	if (_conn_fd < 0) {
		std::cerr << "Failed to accept client request." << std::endl;
		throw GenericException();
	}
	std::cout << "Accepted connection. fd: " << _conn_fd << std::endl;
	struct timeval timeout;
	timeout.tv_sec = 0;  // 5 seconds timeout
	timeout.tv_usec = 20;

	setsockopt(_conn_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
	// fcntl(_conn_fd, F_SETFL, O_NONBLOCK);
	// int	rcvbuf_size;
	// int	sndbuf_size;
	// socklen_t len = sizeof(rcvbuf_size);
	//    if (getsockopt(_conn_fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, &len) < 0) {
	//        close(_conn_fd);
	//        return ;
	//    }
	// std::cout << "SO_RCVBUF (Receive Buffer Capacity): " << rcvbuf_size << std::endl;
	//
	//    // Get Send Buffer Size
	//    len = sizeof(sndbuf_size);
	//    if (getsockopt(_conn_fd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, &len) < 0) {
	//        close(_conn_fd);
	//        return ;
	//    }
	// std::cout << "SO_SNDBUF (Receive Buffer Capacity): " << sndbuf_size << std::endl;
}

void	Worker::setSrv(TCPServer *srv)
{
	_srv = srv;
}

void logServingFile(const std::string& path, const std::string& mimetype) {
	std::cout << "Serving file: " << path << " with MIME type: " << mimetype << std::endl;
}

size_t	Worker::extract_body(size_t nread, size_t old_size, size_t clcr_pos) const
{
	const size_t	body_start = clcr_pos + 4 - old_size;
	const size_t	body_size = nread - body_start;
	const char		*src = &_req_buffer[0] + body_start;

	_req->body.resize(body_size);
	std::memcpy(_req->body.data(), src, body_size);
	return (body_size);
}

void	Worker::parse_range(HttpResponse& res) const
{
	std::string	rangestr = _req->headers["range"];
	size_t		i = rangestr.find('=');
	char		*endptr;
	size_t		start = std::strtol(&rangestr.c_str()[i + 1], &endptr, 10);

	if (*endptr != '-')
		return ;
	endptr++;
	size_t		end = std::strtol(endptr, &endptr, 10);
	if (end == 0)
		end = res.body.length() - 1;
	res.headers["content-range"] = "bytes " + ::itoa(start) + "-" + ::itoa(end) + "/" + ::itoa(res.body.length());
	if (end < res.body.length() - 1)
		res.body.erase(end + 1);
	res.body.erase(0, start);
}

int Worker::handleRequest()
{
	int			nread;

	nread = read(_conn_fd, _req_buffer, REQUEST_BUF_SIZE);
	if (nread <= 0)
		return (2);
	_req_buffer[nread] = '\0';
	switch (_status) {
		case (REQ_HEADERS): {
			size_t	old_size = _rawRequest.size();
			_rawRequest += _req_buffer;

			size_t	clcr_pos = _rawRequest.find("\r\n\r\n");
			if (clcr_pos == _rawRequest.npos)
				return (1);
			std::cout << FT_MAGENTA << "Request ready on fd: " << _conn_fd << std::endl;
			std::cout << FT_GREEN << _rawRequest.substr(0, clcr_pos + 2) << FT_RESET << std::endl;
			_req = new HttpRequest();
			try {
				_req->parseRequest(_rawRequest);
			}
			catch (HttpRequest::GenericException &e) {
				std::cout << e.what() << std::endl;
				_srv->requests_failed++;
				_rawRequest.erase();
				delete _req;
				return (0);
			}
			_req->content_length = std::atoi(_req->headers["content-length"].c_str());
			if (_req->content_length > 0)
			{
				_status = REQ_BODY;
				size_t	body_size = extract_body(nread, old_size, clcr_pos);
				if (body_size < _req->content_length)
					return (1);
				_req->printBody();
			}
			break ;
		}
		case (REQ_BODY): {
			size_t old_size = _req->body.size();
			_req->body.resize(old_size + nread);
			std::memcpy(_req->body.data() + old_size, _req_buffer, nread);
			if (_req->body.size() < _req->content_length)
				return (1);
			_req->printBody();
			break ;
		}
		default:
			break ;
	}

	_res = Worker::prepareResponse();
	_res->buildHttpResponse();

	setStatus(REQ_RESPONSE_READY);

	return (0);
}

void	resize_socket_buffer(int sockfd, size_t size)
{
	int	sndbuf_size;
	socklen_t len = sizeof(sndbuf_size);

	getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, &len);
	std::cout << "Old sock_buf_size: " << sndbuf_size << std::endl;
	if (size < static_cast<size_t>(sndbuf_size))
		return ;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, len);
	getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, &len);
	std::cout << "New sock_buf_size: " << sndbuf_size << std::endl;
}

int	Worker::sendResponse(void)
{
	std::string& response = _res->response;

	if (_res->start == 0)
	{
		std::string& type = _res->headers["content-type"];
		if (!_res->body.empty())
			logServingFile(_res->filename, type);
		if (type.substr(0, type.find_first_of("/")) == "text")
			std::cout << FT_BLUE << response << FT_RESET << std::endl;
		else
			std::cout << FT_BLUE << response.substr(0, response.find("\r\n\r\n")) << "\n<Binary file>" << FT_RESET << std::endl;
	}
	// size_t	msg_size = RESPONSE_MSG_SIZE;
	size_t	msg_size = response.length() - _res->start;
	// resize_socket_buffer(_conn_fd, msg_size);
	if (msg_size > response.length() - _res->start)
		msg_size = response.length() - _res->start;
	msg_size = write(_conn_fd, &response.c_str()[_res->start], msg_size);
	std::cout << "on fd: " << _conn_fd << " wrote: " << msg_size << std::endl;
	_res->start += msg_size;
	if (_res->start >= response.length())
	{
		_srv->requests_handled++;
		_rawRequest.erase();
		reset();
		return (0);
	}
	return (1);
}

void	Worker::handle_error_response(HttpResponse *res) const
{
	std::string path = _srv->getCfg().http.server.error_pages.at(res->statuscode);
	if (path[0] != '.')
	{
		Location *location = loc_trie_search(_srv->getCfg().http.server.loc_trie, path);
		path = apply_location(path, location);
	}
	res->headers["content-type"] = _req->getMimeType(path);
	res->body = res->readHtmlFile(path);
	res->headers["content-length"] = ::itoa(res->body.length());

}

HttpResponse*	Worker::prepareResponse() const
{
	HttpResponse*	res = new HttpResponse();
	Location		*location = loc_trie_search(_srv->getCfg().http.server.loc_trie, _req->path);
	std::string		mimetype;

	res->statuscode = "200";
	res->statusmsg = "OK";
	// res->headers = _req->headers;
	res->headers["Server"] = "Webserv/0.69";

	if (std::find(location->_methods.begin(), location->_methods.end(), _req->method) == location->_methods.end())
	{
		res->statuscode = "405";
		res->statusmsg = "Method Not Allowed";
		handle_error_response(res);
		return res;
	}

	CGI	*cgi = cgi_trie_search(location->cgi_trie, _req->path);
	if (cgi != NULL)
	{
		res->statuscode = "500";
		res->statusmsg = "Internal Server Error";
		res->headers["boop"] = "beep";
		handle_error_response(res);
		return res;

	}

	try {
		if (_req->method == "GET")
		{
			res->body = _req->getHtmlResponse(location, *res);
			// if (res->headers["content-type"] == "video/mp4" )
			// {
				if (_req->headers["range"].empty())
					res->headers["accept-ranges"] = "bytes";
				else if (_req->headers["range"].find("bytes") == 0)
				{
					parse_range(*res);
					res->statuscode = "206";
					res->statusmsg = "Partial Content";
				}
			// }
			res->headers["content-length"] = ::itoa(res->body.length());
		}
		else if (_req->method == "PUT")
		{
			std::string	rel_path = _req->path.substr(location->_path.length(), _req->path.length());
			if (rel_path.empty())
				rel_path = "default";
			std::string	path = location->_root + rel_path;
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
			int	fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
			write(fd, _req->body.data(), _req->body.size());
			close(fd);
		}
		else if (_req->method == "DELETE")
		{

		}
	} catch (std::exception &e)
	{
		handle_error_response(res);
	}

	return (res);
}

const char *Worker::GenericException::what() const throw()
{
	return "Client exception happened";
}

// Worker&	Worker::operator=(Worker const &src)
// {
// 	this->_socket_fd = src._socket_fd;
// 	this->srv = src.srv;
// 	return (*this);
// }

int Worker::getConnFd() const
{
	return _conn_fd;
}

void	Worker::closeSocketFd(void)
{
	close(_conn_fd);
	_conn_fd = -1;
}

int Worker::requestHandled() const
{
	return _request_handled;
}

std::string& Worker::getRawRequest()
{
	return _rawRequest;
}

void	Worker::setReq(HttpRequest* req)
{
	_req = req;
}

void	Worker::setRes(HttpResponse* res)
{
	_res = res;
}

HttpResponse*	Worker::getRes() const
{
	return _res;
}

HttpRequest*	Worker::getReq() const
{
	return _req;
}

void	Worker::clearRequest(void)
{
	if (!_rawRequest.empty())
		_rawRequest.erase();
}

void	Worker::setStatus(e_status status)
{
	_status = status;
}

void	Worker::reset()
{
	delete _res;
	delete _req;
	_res = NULL;
	_req = NULL;
	_status = REQ_HEADERS;
}

Worker::e_status	Worker::getStatus(void) const
{
	return (_status);
}
