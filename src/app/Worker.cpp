/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:02:20 by abelov            #+#    #+#             */
/*   Updated: 2025/08/23 20:29:15 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"

#include <cassert>

#include "webserv.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <cstdlib>
#include <unistd.h>

Worker::Worker(TCPServer &srv)
	: _req_buffer(), _req(),
	_conn_fd(-1),
	_request_handled(),
	_addr(),
	_addr_size(),
	srv(srv)
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
	_conn_fd = /* global namespace */ ::accept(srv.getSocketFd(), addr, &_addr_size);
	if (_conn_fd < 0) {
		std::cerr << "Failed to accept client request." << std::endl;
		throw GenericException();
	}
	std::cout << "Accepted connection. fd: " << _conn_fd << std::endl;
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

int Worker::handleRequest()
{
	int			nread;

	nread = read(_conn_fd, _req_buffer, 1023);
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
				srv.requests_failed++;
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

	HttpResponse* res = Worker::prepareResponse();

	std::string response = res->buildHttpResponse();
	std::string& type = res->headers["content-type"];

	if (!res->body.empty())
		logServingFile(res->filename, type);
	if (type.substr(0, type.find_first_of("/")) == "text")
		std::cout << FT_BLUE << response << FT_RESET << std::endl;
	else
		std::cout << FT_BLUE << response.substr(0, response.find("\r\n\r\n")) << "\n<Binary file>" << FT_RESET << std::endl;
	write(_conn_fd, response.c_str(), response.length());
	srv.requests_handled++;
	// close(_socket_fd);
	_rawRequest.erase();
	delete _req;
	setReq(NULL);
	delete res;
	return (0);
}

HttpResponse*	Worker::prepareResponse() const
{
	HttpResponse*	res = new HttpResponse();
	std::string		mimetype;

	res->statuscode = "200";
	res->statusmsg = "OK";
	// res->headers = _req->headers;
	res->headers["Server"] = "Webserv/0.69";

	if (_req->method == "GET")
		res->body = _req->getHtmlResponse(srv.getCfg(), *res);
	else if (_req->method == "PUT")
	{
		std::string	rel_path = _req->path.substr(1, _req->path.length());
		if (rel_path.empty())
			rel_path = "default";
		std::string	path = srv.getCfg().http.server.location.config.root + "/put_test/" + rel_path;
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
		int	fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
		write(fd, _req->body.data(), _req->body.size());
		close(fd);
	}
	else if (_req->method == "DELETE")
	{

	}

	res->headers["content-length"] = ::itoa(res->body.length());
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

int Worker::getSocketFd() const
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

void	Worker::clearRequest(void)
{
	if (!_rawRequest.empty())
		_rawRequest.erase();
}
