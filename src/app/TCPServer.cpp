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
#include "ConfigParser.hpp"
#include "Worker.hpp"
#include "WorkerPool.hpp"
#include "serve.hpp"

/*
** -------------------------------- STATIC VARS -------------------------------
*/

Config TCPServer::DEFAULT_CONFIG = Parser::make_default_config();
/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/


TCPServer::TCPServer(const Config& conf) : cfg(conf)
{

}

TCPServer::TCPServer() : cfg(DEFAULT_CONFIG)
{

}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/

TCPServer::~TCPServer()
{
    std::cout << "Requests handled: " << requests_handled << std::endl;
    std::cout << "Requests failed: " << requests_failed << std::endl;
}

/*
** -------------------------------- OPERATORS ---------------------------------
*/


/*
** --------------------------------- METHODS ----------------------------------
*/

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

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



void TCPServer::stop()
{
	close(_socket_fd);
	free_loc_trie(this->getCfg().http.server.loc_trie);
}

int TCPServer::getSocketFd() const
{
    return _socket_fd;
}

const Config &TCPServer::getCfg() const
{
	return cfg;
}

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

int TCPServer::serve(TCPServer &srv)
{
	extern sig_atomic_t				g_var;
	WorkerPool						wrkrPool(srv.WRKR_POOL_SIZE);
	int								epoll_fd = epoll_create(1);
	std::vector<struct epoll_event>	evs;
	int								nfds;
	int								sockfd = srv.getSocketFd();
	Worker*							wrkr;

	evs.resize(1024);
	evs[0].events = EPOLLIN;
	evs[0].data.fd = sockfd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &evs[0]);

	while(g_var != SIGINT)
	{
		nfds = epoll_wait(epoll_fd, evs.data(), 1024, -1);
		for (int i = 0; i < nfds; i++)
		{
			wrkr = reinterpret_cast<Worker*>(evs[i].data.ptr);
			// if (evs[i].data.fd == sockfd)
			// 	std::cout << "event on fd: " << sockfd << std::endl;
			// else
			// 	std::cout << "event on fd: " << wrkr->getConnFd() << std::endl;
			if (evs[i].events & (EPOLLERR | EPOLLHUP))
			{
				std::cout << "error occured on fd: " << wrkr->getConnFd() << std::endl;
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, wrkr->getConnFd(), NULL);
				wrkr->reset();
				wrkrPool.free(wrkr);
			}
			else if (evs[i].events & EPOLLOUT && wrkr->getStatus() == Worker::REQ_RESPONSE_READY)
			{
				// std::cout << "Write ready on fd: " << wrkr->getConnFd() << std::endl;
				int retval = wrkr->sendResponse();
				if (retval == 0)
				{
					struct epoll_event ev;
					ev.data.ptr = wrkr;
					ev.events = EPOLLIN;
					epoll_ctl(epoll_fd, EPOLL_CTL_MOD, wrkr->getConnFd(), &ev);
				}
			}
			else if (evs[i].events & EPOLLIN)
			{
				// std::cout << "Read ready on fd " << std::endl;
				if (evs[i].data.fd == sockfd)
					assignWorker(wrkrPool, epoll_fd); // <==
				else
				{
					int retval = wrkr->handleRequest();
					if (wrkr->getStatus() == Worker::REQ_RESPONSE_READY)
					{
						struct epoll_event ev;
						ev.data.ptr = wrkr;
						ev.events = EPOLLOUT;
						epoll_ctl(epoll_fd, EPOLL_CTL_MOD, wrkr->getConnFd(), &ev);
					}
					if (retval == 2)
					{
						std::cout << "Connection closed on fd: " << wrkr->getConnFd() << std::endl;
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, wrkr->getConnFd(), NULL);
						wrkr->reset();
						wrkrPool.free(wrkr);
					}
				}
			}
		}
	}
	close(epoll_fd);
	return 0;
}

const char *TCPServer::GenericException::what() const throw()
{
	return "Server exception happened";
}
