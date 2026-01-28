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
    TCPServer();

protected:
	const Config &cfg;
public:
    static const unsigned int WRKR_POOL_SIZE = 1024;
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
	void assignWorker(WorkerPool& wrkrPool, int epoll_fd);
	void stop();
	unsigned long	requests_handled;
	unsigned long	requests_failed;

	int serve(TCPServer &srv);
};


#endif //TCPSERVER_HPP
