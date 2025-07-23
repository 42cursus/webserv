/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:12:05 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 19:12:05 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <sys/wait.h>
#include "webserv.hpp"

class Server
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
	explicit Server(Config conf);
	Server();
	~Server();

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};
	int getSocketFd() const;
	int start();
	void stop();

};


#endif //SERVER_HPP
