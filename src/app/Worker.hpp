/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:31:02 by abelov            #+#    #+#             */
/*   Updated: 2025/07/23 21:16:43 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WORKER_HPP
#define WORKER_HPP


#include <netinet/in.h>
#include "TCPServer.hpp"

class Worker
{
private:
	char _req_buffer[1024];
	int _socket_fd;
	struct sockaddr_in _addr;
	socklen_t _addr_size;
	TCPServer &srv;
public:
	explicit Worker(TCPServer &);
	~Worker();

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};

	void acceptConnection();
	void handleRequest();
};


#endif //WORKER_HPP
