/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:12:40 by abelov            #+#    #+#             */
/*   Updated: 2026/02/03 00:26:36 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TCPServer.hpp"
#include "ConnWorker.hpp"
#include "HttpRequest.hpp"
#include "Logging.hpp"
#include "Prefix_suffix.hpp"
#include "WebServer.hpp"
#include "WorkerPool.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>

/*
** -------------------------------- STATIC VARS -------------------------------
*/

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/


TCPServer::TCPServer(const Config& conf) : cfg(conf)
{

}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/

TCPServer::~TCPServer()
{
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
	sockaddr_in in = cfg.http.server.ipv4_listen;

	_socket_fd = socket(in.sin_family, SOCK_STREAM, 0);

	if (_socket_fd < 0) {
		std::cerr << "Failed to create server socket." << std::endl;
		throw TCPServer::GenericException();
	}
    int flags = fcntl(_socket_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        std::cerr << "Failed to set O_NONBLOCK on listen socket: " << strerror(errno) << std::endl;
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
			  << "http://" << inet_ntoa(inin.sin_addr) << ":" << ntohs(in.sin_port) << "/\n"
			  << std::endl;
	return _socket_fd;
}

void TCPServer::stop()
{
	close(_socket_fd);
	free_trie(this->getCfg().http.server.loc_trie);
	free_trie(this->getCfg().http.server.redirect_trie);
}

int TCPServer::getSocketFd() const
{
    return _socket_fd;
}

const Config &TCPServer::getCfg() const
{
	return cfg;
}

void	TCPServer::acceptAllPendingConns(WorkerPool& wrkrPool, int epoll_fd)
{
    ConnWorker * wrkr;

    while (true) // multiple connections may already be queued on the listen socket
    {
        struct sockaddr_in		_addr;
        socklen_t				_addr_size = sizeof(_addr);
        struct sockaddr         *addr = reinterpret_cast<struct sockaddr*>(&_addr); // NOLINT(*-pro-type-reinterpret-cast)

		// char buf[1024] = {0};
		// read(_socket_fd, buf, 1023);
		// std::cout << "-----------------------------------------------" << std::endl;
		// std::cout << buf << std::endl;
		// printf("%m\n");
		// exit(0);
        int conn_fd = ::accept(_socket_fd, addr, &_addr_size);
		// std::cout << inet_ntoa(_addr.sin_addr) << ":" << ntohs(_addr.sin_port) << std::endl;
        if (conn_fd < 0) {
			// Something fundamentally wrong happened
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            std::cerr << "Failed to accept client request." << std::endl;
            break;
        }

        wrkr = wrkrPool.alloc(this);
        wrkr->setConnFd(conn_fd);
		std::string ip = inet_ntoa(_addr.sin_addr);
		ip += ":" + ::itoa(ntohs(_addr.sin_port));
		const_cast<Connection&>(wrkr->getConn()).setIpStr(ip);

        // std::cout << "Accepted connection. fd: " << conn_fd << std::endl;
		log_connection(*wrkr, CONNECT);
        struct timeval timeout;
        timeout.tv_sec = 0;  // 5 seconds timeout
        timeout.tv_usec = 20;

        setsockopt(conn_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

        struct epoll_event ev;
        std::memset(&ev, 0, sizeof(ev));
        ev.data.ptr = tag_ptr(wrkr, WebServer::EP_WRKR);
        ev.events = EPOLLIN;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
    }
};

const char *TCPServer::GenericException::what() const throw()
{
	return "Server exception happened";
}
