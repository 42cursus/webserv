/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:12:05 by abelov            #+#    #+#             */
/*   Updated: 2025/07/23 21:00:53 by abelov           ###   ########.fr       */
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

class TCPServer
{
private:
	static const unsigned int DEFAULT_PORT = 8080;
	int _socket_fd;

protected:
	const Config cfg;
public:
	const Config &getCfg() const;

public:
	static Config default_config;
	explicit TCPServer(Config conf);
	TCPServer();
	~TCPServer();

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};
	int getSocketFd() const;
	int start();
	int serve(TCPServer &);
	void stop();
};


#endif //TCPSERVER_HPP
