/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:02:20 by abelov            #+#    #+#             */
/*   Updated: 2025/07/23 21:16:48 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"
#include "src/http/HttpRequest.hpp"
#include "src/http/HttpResponse.hpp"

#ifdef __clang__
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

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
}

void logServingFile(const std::string& path, const std::string& mimetype) {
	std::cout << "Serving file: " << path << " with MIME type: " << mimetype << std::endl;
}

void Worker::handleRequest()
{
	read(_socket_fd, _req_buffer, 1024);
	HttpRequest req = HttpRequest();

	req.parseRequest(_req_buffer);
	std::map<const std::string, std::string>::iterator it = req.headers.begin();
	while (it != req.headers.end())
	{
		std::cout << it->first << " : " << it->second << std::endl;
		it++;
	}
	std::string mimetype = req.getMimeType(req.path);
	std::string body = req.readHtmlFile(req.path, srv.getCfg().http.server.location.config.root);

	HttpResponse res= HttpResponse();

	std::string response = res.buildHttpResponse("200", "OK", req.headers, body, mimetype);
	logServingFile(req.path, mimetype);

	write(_socket_fd, response.c_str(), response.length());
	close(_socket_fd);
}

const char *Worker::GenericException::what() const throw()
{
	return "Client exception happened";
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
