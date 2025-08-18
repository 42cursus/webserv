/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:12:40 by abelov            #+#    #+#             */
/*   Updated: 2025/07/23 21:01:03 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <map>
#include <sys/select.h>
#include <poll.h>
#include <vector>
#include "TCPServer.hpp"
#include "src/utils/Parser.hpp"
#include "Worker.hpp"

Config TCPServer::default_config = Parser::make_default_config();

TCPServer::TCPServer(const Config conf) : cfg(conf)
{

}

TCPServer::TCPServer() : cfg(default_config)
{

}

int TCPServer::start()
{
	sockaddr_in in = cfg.http.server.ipv4_listen;
	_socket_fd = socket(in.sin_family, SOCK_STREAM, 0);
	if (_socket_fd < 0) {
		std::cerr << "Failed to create server socket." << std::endl;
		throw TCPServer::GenericException();
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
		throw TCPServer::GenericException();
	}
	// listens on socket
	if (listen(_socket_fd, 5) < 0)
	{
		std::cerr << "Failed to listen on server socket." << std::endl;
		throw TCPServer::GenericException();
	}
	std::cout << "Server started on port: " << ntohs(in.sin_port) << std::endl;
	std::cout << "Listen socket_fd: " << _socket_fd << std::endl;
	return _socket_fd;
}

TCPServer::~TCPServer()
{

}

int TCPServer::getSocketFd() const
{
	return _socket_fd;
}

void TCPServer::stop()
{
	close(_socket_fd);
}

const Config &TCPServer::getCfg() const
{
	return cfg;
}

// int TCPServer::serve(TCPServer &srv)
// {
// 	extern sig_atomic_t g_var;
//
// 	while(g_var != SIGINT)
// 	{
// 		Worker wrkr(srv);
// 		wrkr.acceptConnection();
// 		wrkr.handleRequest();
// 	}
// 	return 0;
// }

int TCPServer::serve(TCPServer &srv)
{
	extern sig_atomic_t		g_var;
	std::map<int , Worker*>	connections;

	while(g_var != SIGINT)
	{
		std::vector<struct pollfd>	pollfds;
		pollfds.push_back((struct pollfd){.fd = srv.getSocketFd(), .events = POLLIN});
		std::map<int , Worker*>::iterator it = connections.begin();
		while (it != connections.end())
		{
			pollfds.push_back((struct pollfd){.fd = it->first, .events = POLLIN});
			it++;
		}
		// std::cout << "pollfds: ";
		// for (size_t i = 0; i < pollfds.size(); i++)
		// 	std::cout << pollfds[i].fd << ' ';
		// std::cout << std::endl;
		poll(pollfds.data(), pollfds.size(), -1);
		if (pollfds[0].revents & POLLIN)
		{
			Worker* wrkr = new Worker(srv);
			wrkr->acceptConnection();
			connections[wrkr->getSocketFd()] = wrkr;
		}
		for (size_t i = 1; i < pollfds.size(); i++)
		{
			if (pollfds[i].revents & POLLIN)
			{
				int	fd = pollfds[i].fd;
				connections[fd]->handleRequest();
				if (connections[fd]->requestHandled())
				{
					delete connections[fd];
					connections.erase(fd);
				}
			}
		}
	}
	return 0;
}

const char *TCPServer::GenericException::what() const throw()
{
	return "Server exception happened";
}
