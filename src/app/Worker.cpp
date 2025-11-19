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

Worker::Worker(TCPServer &srv)
	: _req_buffer(),
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

int Worker::handleRequest()
{
	int			nread;

	nread = read(_socket_fd, _req_buffer, 1023);
	if (nread <= 0)
		return (2);
	_req_buffer[nread] = '\0';
	_rawRequest += _req_buffer;
	if (_rawRequest.find("\r\n\r\n") == _rawRequest.npos)
		return (1);
	std::cout << "\e[35m" << "Request ready on fd: " << _socket_fd << std::endl;
	HttpRequest req = HttpRequest();
	std::cout << "\e[32m" << _rawRequest << "\e[31m" << std::endl;
	// for (int i = 0; _rawRequest[i] != 0 && i < 1024; i++)
	// {
	// 	std::cout << (int)_rawRequest[i] << ' ';
	// 	if (_rawRequest[i] == '\n')
	// 		std::cout << std::endl;
	// }
	std::cout << "\e[m" << std::endl;

	try {
		req.parseRequest(_rawRequest);
	}
	catch (HttpRequest::GenericException &e) {
		std::cout << e.what() << std::endl;
		_rawRequest.erase();
		return (0);
	}
	std::map<const std::string, std::string>::iterator it = req.headers.begin();
	while (it != req.headers.end())
	{
		std::cout << it->first << " : " << it->second << std::endl;
		it++;
	}
	std::string mimetype = req.getMimeType(req.path);

	std::string body = req.getHtmlResponse(srv.getCfg());

	HttpResponse res = HttpResponse();

	std::string response = res.buildHttpResponse("200", "OK", req.headers, body, mimetype);
	logServingFile(req.path, mimetype);

	write(_socket_fd, response.c_str(), response.length());
	// close(_socket_fd);
	_rawRequest.erase();
	return (0);
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
	_rawRequest.erase();
}
