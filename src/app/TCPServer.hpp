/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:12:05 by abelov            #+#    #+#             */
/*   Updated: 2026/01/22 15:51:43 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <sys/wait.h>
#include "webserv.hpp"

class WorkerPool;

class TCPServer
{
private:
	static const unsigned int DEFAULT_PORT = 8080;
	int _socket_fd;
    static Config DEFAULT_CONFIG;

protected:
	const Config &cfg;

public:
	const Config &getCfg() const;

	explicit TCPServer(const Config& conf);

	~TCPServer();

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};
	int getSocketFd() const;
	int start();
	void acceptAllPendingConns(WorkerPool& wrkrPool, int epoll_fd);
	void stop();
};


#endif //TCPSERVER_HPP
