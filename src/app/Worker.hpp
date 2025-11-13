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


#include <array>
#include <netinet/in.h>
#include <sys/types.h>
#include <vector>
#include "TCPServer.hpp"
#include "src/http/HttpRequest.hpp"

enum {
	REQ_HEADERS = 0,
	REQ_BODY,
	REQ_HANDLED,
	REQ_MAX,
};

class Worker
{
private:
	std::array<char, 1024>	_req_buffer;
	std::string				_rawRequest;
	HttpRequest*			_req;
	int						_req_status;
	int						_socket_fd;
	int						_request_handled;
	struct sockaddr_in		_addr;
	socklen_t				_addr_size;
	TCPServer				&srv;
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
	size_t	extract_body(size_t nread);
};


#endif //WORKER_HPP
