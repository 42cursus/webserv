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

#pragma once
#ifndef WORKER_HPP
#define WORKER_HPP

#include <netinet/in.h>
#include "HttpResponse.hpp"
#include "TCPServer.hpp"
#include "HttpRequest.hpp"

#define REQUEST_BUF_SIZE 4096
#define RESPONSE_MSG_SIZE 4096

class Worker
{
public:

	enum e_status {
		REQ_HEADERS = 0,
		REQ_BODY,
		REQ_RESPONSE_READY,
		REQ_MAX,
	};

private:
	char					_req_buffer[REQUEST_BUF_SIZE + 1];
	std::string				_rawRequest;
	HttpRequest*			_req;
	HttpResponse*			_res;
	int						_conn_fd;
	int						_epoll_fd;
	int						_request_handled;
	struct sockaddr_in		_addr;
	socklen_t				_addr_size;
	TCPServer				&srv;
	e_status				_status;
	std::vector<class Connection> _conns;
public:
	explicit Worker(TCPServer &);
	~Worker();

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};

	void acceptConnection();
	int handleRequest();
	HttpResponse* prepareResponse() const;
	HttpRequest* getReq() const;
	void setReq(HttpRequest* req);
	HttpResponse* getRes() const;
	void setRes(HttpResponse* res);
	int getConnFd() const;
	void	closeSocketFd();
	std::string& getRawRequest();
	int requestHandled() const;
	void clearRequest();
	e_status getStatus(void) const;
	void	setStatus(e_status status);
	size_t	extract_body(size_t nread, size_t old_size, size_t clcr_pos) const;
	void	parse_range(HttpResponse& res) const;
	int	sendResponse();
	void	reset();
};

#endif //WORKER_HPP
