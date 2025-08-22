/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:12:40 by abelov            #+#    #+#             */
/*   Updated: 2025/08/20 22:17:40 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <map>
#include <sys/poll.h>
#include <sys/select.h>
#include <poll.h>
#include <vector>
#include "TCPServer.hpp"
#include "src/utils/Parser.hpp"
#include "Worker.hpp"
#include "WorkerPool.hpp"

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
	extern sig_atomic_t			g_var;
	std::map<int , Worker*>		connections;
	WorkerPool					wrkrPool(srv);
	std::vector<struct pollfd>	pollfds;
	size_t						nfds;

	pollfds.resize(1024);
	pollfds.data()[0] = (struct pollfd){.fd = srv.getSocketFd(), .events = POLLIN, .revents = 0};
	while(g_var != SIGINT)
	{
		std::map<int , Worker*>::iterator it = connections.begin();
		for (nfds = 1; it != connections.end(); it++, nfds++)
		{
			if (nfds == pollfds.size())
				pollfds.resize(pollfds.size() + 1024);
			pollfds.data()[nfds] = (struct pollfd){
				.fd = it->first,
				.events = POLLIN,
				.revents = 0
			};
		}

		poll(pollfds.data(), nfds, -1);
		if (pollfds[0].revents & POLLIN)
		{
			Worker* wrkr = wrkrPool.alloc();
			wrkr->acceptConnection();
			connections[wrkr->getSocketFd()] = wrkr;
		}
		for (size_t i = 1; i < nfds; i++)
		{
			if (pollfds[i].revents & POLLIN)
			{
				int	fd = pollfds[i].fd;
				if (connections[fd]->handleRequest() == 0)
				{
					wrkrPool.free(connections[fd]);
					connections.erase(fd);
				}
			}
			if (pollfds[i].revents & (POLLHUP | POLLERR))
				std::cout << "error occured on fd: " << pollfds[i].fd << std::endl;
		}
	}
	std::map<int , Worker*>::iterator it = connections.begin();
	while (it != connections.end())
	{
		close(it->first);
		it++;
	}
	return 0;
}

const char *TCPServer::GenericException::what() const throw()
{
	return "Server exception happened";
}
