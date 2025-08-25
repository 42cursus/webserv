/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:31:02 by abelov            #+#    #+#             */
/*   Updated: 2025/08/23 20:29:23 by fsmyth           ###   ########.fr       */
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
	std::string _rawRequest;
	int _socket_fd;
	int _request_handled;
	struct sockaddr_in _addr;
	socklen_t _addr_size;
	TCPServer &srv;
public:
	explicit Worker(TCPServer &);
	~Worker();
	// Worker&	operator=(Worker const &src);

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};

	void acceptConnection();
	int handleRequest();
	int getSocketFd() const;
	std::string& getRawRequest();
	int requestHandled() const;
	void clearRequest();
};


#endif //WORKER_HPP
