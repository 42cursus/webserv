/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:02:20 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 21:02:21 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "src/http/HttpRequest.hpp"
#include "src/http/HttpResponse.hpp"

Client::Client() : _socket_fd(), _addr(), _addr_size()
{

}

Client::~Client()
{

}

void Client::acceptConnection(Server &srv)
{
	_socket_fd = accept(srv.getSocketFd(), (struct sockaddr*)&_addr, &_addr_size);
	if (_socket_fd < 0) {
		std::cerr << "Failed to accept client request." << std::endl;
		throw Client::GenericException();
	}
}

void logServingFile(const std::string& path, const std::string& mimetype) {
	std::cout << "Serving file: " << path << " with MIME type: " << mimetype << std::endl;
}

void Client::handleRequest()
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
	HttpResponse res= HttpResponse();
	std::string body = req.readHtmlFile(req.path);
	std::string response = res.buildHttpResponse("200", "OK", req.headers, body, mimetype);
	logServingFile(req.path, mimetype);

	write(_socket_fd, response.c_str(), response.length());
	close(_socket_fd);
}

const char *Client::GenericException::what() const throw()
{
	return "Client exception happened";
}
