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

Config Server::make_default_config()
{
	Config cfg = {0x00};
	std::memset(&cfg.server_addr, 0, sizeof(cfg.server_addr));
	cfg.server_addr.sin_family = AF_INET;
	cfg.server_addr.sin_port = htons(Server::DEFAULT_PORT);
	cfg.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	return cfg;
}

Config Server::default_config = make_default_config();

Server::Server(const Config conf) : cfg(conf)
{

}

Server::Server() : cfg(default_config)
{

}

int Server::start()
{
	_socket_fd = socket(cfg.server_addr.sin_family, SOCK_STREAM, 0);
	if (_socket_fd < 0) {
		std::cerr << "Failed to create server socket." << std::endl;
		throw Server::GenericException();
	}
	/* bind  socket to port */
	if (bind(_socket_fd, (struct sockaddr*)&cfg.server_addr, sizeof(cfg.server_addr)) < 0) {
		std::cerr << "Failed to bind server socket." << std::endl;
		throw Server::GenericException();
	}
	/* listens on socket */
	if (listen(_socket_fd, 5) < 0) {
		std::cerr << "Failed to listen on server socket." << std::endl;
		throw Server::GenericException();
	}
	std::cout << "Server started on port: " << ntohs(cfg.server_addr.sin_port) << std::endl;
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

}

const char *Server::GenericException::what() const throw()
{
	return "Server exception happened";
}
