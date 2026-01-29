/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnWorker.hpp                                         :+:      :+:    :+:   */
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
#define CRLF "\r\n"

class ConnWorker {
public:

	enum e_status {
		REQ_HEADERS = 0,
		REQ_BODY,
		REQ_RESPONSE_READY,
		REQ_MAX,
	};

	explicit ConnWorker();
	~ConnWorker();

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};

	void acceptConnection();

    int onWritable();
    int onReadable();

    void setConnFd(int connFd);

	HttpRequest* getReq() const;
	void setReq(HttpRequest* req);
	HttpResponse* getRes() const;
	void setRes(HttpResponse* res);
	int getConnFd() const;
	void	setSrv(TCPServer *srv);
	void	closeSocketFd();
	std::string& getRawRequest();
	void clearRequest();

	e_status getStatus() const;
	void	setStatus(e_status status);

    void	reset();

private:
    char					_req_buffer[REQUEST_BUF_SIZE + 1];
    std::string				_rawRequest;
    HttpRequest*			_req;
    HttpResponse*			_res;
    int						_conn_fd;
    TCPServer				*_srv;
    e_status				_status;

    size_t	extract_body(size_t nread, size_t old_size, size_t clcr_pos) const;
    void	parse_range(HttpResponse& res) const;
    void	handle_error_response(HttpResponse *res) const;
    HttpResponse* prepareResponse() const;
};

#endif //WORKER_HPP
