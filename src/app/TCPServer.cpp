/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:12:40 by abelov            #+#    #+#             */
/*   Updated: 2026/01/22 15:51:35 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>
#include "TCPServer.hpp"
#include "HttpRequest.hpp"
#include "Prefix.hpp"
#include "Worker.hpp"
#include "WorkerPool.hpp"
#include "serve.hpp"


TCPServer::TCPServer(const Config& conf) : cfg(conf)
{

}

// TCPServer::TCPServer() : cfg(default_config)
// {
//
// }

int TCPServer::start()
{
	requests_handled = 0;
	requests_failed = 0;
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

	struct sockaddr_in inin = in;
	inin.sin_family = in.sin_family;
	if (ntohs(in.sin_addr.s_addr) == htonl(INADDR_ANY))
		inin.sin_addr.s_addr = inet_addr("127.0.0.1");

	std::cout << "Listen socket_fd: " << _socket_fd << std::endl;
	std::cout << "Server started on: "
			  << "http://" << inet_ntoa(inin.sin_addr)
			  << ":" << ntohs(in.sin_port) << "/\n"
			  << std::endl;
	return _socket_fd;
}

TCPServer::~TCPServer()
{
	std::cout << "Requests handled: " << requests_handled << std::endl;
	std::cout << "Requests failed: " << requests_failed << std::endl;
}

int TCPServer::getSocketFd() const
{
	return _socket_fd;
}

void TCPServer::stop()
{
	close(_socket_fd);
	free_loc_trie(this->getCfg().http.server.loc_trie);
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

void	TCPServer::assignWorker(WorkerPool& wrkrPool, int epoll_fd)
{
	Worker* wrkr;

	// if (wrkrPool.getNumAlloced() > 900)
	// 	continue ;
	wrkr = wrkrPool.alloc(this);
	wrkr->acceptConnection();
	struct epoll_event ev;
	// wrkr->setReq(new HttpRequest());
	ev.data.ptr = tag_ptr(wrkr, EP_WRKR);
	ev.events = EPOLLIN;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, wrkr->getConnFd(), &ev);
};

const char *TCPServer::GenericException::what() const throw()
{
	return "Server exception happened";
}
