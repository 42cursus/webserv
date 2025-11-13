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
#include "src/http/HttpRequest.hpp"
#include "src/http/HttpResponse.hpp"
#include <array>
#include <sys/types.h>

Worker::Worker(TCPServer &srv)
	: _req_buffer(),
	_req_status(REQ_BODY),
	_socket_fd(),
	_addr(),
	_addr_size(),
	srv(srv)
{

}

Worker::~Worker()
{

}

void Worker::acceptConnection()
{
	_socket_fd = accept(srv.getSocketFd(), (struct sockaddr*)&_addr, &_addr_size);
	if (_socket_fd < 0) {
		std::cerr << "Failed to accept client request." << std::endl;
		throw Worker::GenericException();
	}
	std::cout << "Accepted connection. fd: " << _socket_fd << std::endl;
}

void logServingFile(const std::string& path, const std::string& mimetype) {
	std::cout << "Serving file: " << path << " with MIME type: " << mimetype << std::endl;
}

size_t	Worker::extract_body(size_t nread)
{
	std::array<char, 1024>::iterator	it = _req_buffer.begin();
	size_t								consumed;
	
	while ((consumed = std::distance(_req_buffer.begin(), it + 3)) < nread)
	{
		if (*it == '\r' && *(it + 1) == '\n'
			&& *(it + 2) == '\r' && *(it + 3) == '\n')
		{
			_req->body.insert(_req->body.begin(), it + 4, _req_buffer.begin() + nread);
			return (nread - consumed);
		}
		it++;
	}
	return (-1);
}

int Worker::handleRequest()
{
	int			nread;

	nread = read(_socket_fd, _req_buffer.data(), 1023);
	if (nread <= 0)
		return (2);
	_req_buffer[nread] = '\0';
	switch (_req_status) {
		case (REQ_HEADERS):
			_rawRequest += static_cast<char *>(_req_buffer.data());
			if (_rawRequest.find("\r\n\r\n") == _rawRequest.npos)
				return (1);
			std::cout << "\e[35m" << "Request ready on fd: " << _socket_fd << std::endl;
			std::cout << "\e[32m" << _rawRequest << "\e[31m" << std::endl;
			for (int i = 0; _rawRequest[i] != 0 && i < 1024; i++)
			{
				std::cout << (int)_rawRequest[i] << ' ';
				if (_rawRequest[i] == '\n')
					std::cout << std::endl;
			}
			std::cout << "\e[m" << std::endl;
			_req = new HttpRequest();
			try {
				_req->parseRequest(_rawRequest);
			}
			catch (HttpRequest::GenericException &e) {
				std::cout << e.what() << std::endl;
				_rawRequest.erase();
				delete _req;
				return (0);
			}
			if (std::atoi(_req->headers["content-length"].c_str()) > 0)
			{
				_req_status = REQ_BODY;
				size_t	body_size = extract_body(nread);
				for (size_t i = 0; i < body_size; i++)
					std::cout << _req->body[i];
			}
	}

	HttpResponse res = HttpResponse();
	res.statuscode = "200";
	res.statusmsg = "OK";
	res.headers = _req->headers;
	

	std::map<const std::string, std::string>::iterator it = _req->headers.begin();
	while (it != _req->headers.end())
	{
		std::cout << it->first << " : " << it->second << std::endl;
		it++;
	}
	std::string mimetype = _req->getMimeType(_req->path);
	res.body = _req->getHtmlResponse(srv.getCfg());

	std::string response = res.buildHttpResponse(res.statuscode, res.statusmsg, res.headers, res.body, mimetype);
	logServingFile(_req->path, mimetype);

	write(_socket_fd, response.c_str(), response.length());
	// close(_socket_fd);
	_rawRequest.erase();
	return (2);
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
	return _socket_fd;
}

int Worker::requestHandled() const
{
	return _request_handled;
}

std::string& Worker::getRawRequest()
{
	return _rawRequest;
}

void	Worker::clearRequest(void)
{
	if (!_rawRequest.empty())
		_rawRequest.erase();
	_req_status = REQ_BODY;
}
