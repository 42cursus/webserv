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
	if (_socket_fd < 0){
		std::cerr << "Failed to accept client request." << std::endl;
		throw Client::GenericException();
	}
}

void Client::handleRequest()
{
	//handle client request.
	//read from the client.
	read(_socket_fd, _req_buffer, 1024);
	HttpRequest req = HttpRequest();

	req.parseRequest(_req_buffer);
	// for(auto it = req.headers.begin(); it != req.headers.end(); it++){
	//     std::cout << it->first << " : " << it->second << std::endl;
	// }
	std::string mimetype = req.getMimeType(req.path);
	// std::cout << "Mimetype: " << mimetype << std::endl;
	HttpResponse res= HttpResponse();
	std::string body = req.readHtmlFile(req.path);
	std::string response = res.buildHttpResponse("200", "OK", req.headers, body,
												 mimetype);
//	logServingFile(req.path, mimetype);

	//write to client.
	write(_socket_fd, response.c_str(), response.length());
	//close client socket.
	close(_socket_fd);
}

const char *Client::GenericException::what() const throw()
{
	return "Client exception happened";
}
