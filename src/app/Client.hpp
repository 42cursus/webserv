/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:31:02 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 20:31:03 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP


#include <netinet/in.h>
#include "Server.hpp"

class Client
{
private:
	char _req_buffer[1024];
	int _socket_fd;
	struct sockaddr_in _addr;
	socklen_t _addr_size;
public:
	Client();
	~Client();

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};

	void acceptConnection(Server &srv);
	void handleRequest();
};


#endif //CLIENT_HPP
