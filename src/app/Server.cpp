/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:12:40 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 19:12:41 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include "Server.hpp"
#include "src/utils/Parser.hpp"

Config Server::default_config = Parser::make_default_config();

Server::Server(const Config conf) : cfg(conf)
{

}

Server::Server() : cfg(default_config)
{

}

int Server::start()
{
	sockaddr_in in = cfg.http.server.ipv4_listen;
	_socket_fd = socket(in.sin_family, SOCK_STREAM, 0);
	if (_socket_fd < 0) {
		std::cerr << "Failed to create server socket." << std::endl;
		throw Server::GenericException();
	}
	int reuse = 1;
	int result = setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse));
	if (result < 0)
		std::cerr << "ERROR SO_REUSEADDR:" << strerror(errno) << std::endl;
	/* bind  socket to port */
	int is_bind = bind(_socket_fd, (struct sockaddr *)&in, sizeof in);
	if (is_bind < 0)
	{
		std::cerr << "Failed to bind server socket." << std::endl;
		throw Server::GenericException();
	}
	// listens on socket
	if (listen(_socket_fd, 5) < 0)
	{
		std::cerr << "Failed to listen on server socket." << std::endl;
		throw Server::GenericException();
	}
	std::cout << "Server started on port: " << ntohs(in.sin_port) << std::endl;
	return _socket_fd;
}

Server::~Server()
{

}

int Server::getSocketFd() const
{
	return _socket_fd;
}

void Server::stop()
{
	close(_socket_fd);
}

const Config &Server::getCfg() const
{
	return cfg;
}

const char *Server::GenericException::what() const throw()
{
	return "Server exception happened";
}
