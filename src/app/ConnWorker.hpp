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

#include "ConnectionContext.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "TCPServer.hpp"
#include <netinet/in.h>

class ConnWorker {
public:
	typedef Connection::e_status e_status;

	explicit ConnWorker();
	ConnWorker(const ConnWorker &other);
	~ConnWorker();

	void setSrv(TCPServer *srv);
	void setConnFd(int connFd);

	int getConnFd() const;

	Connection::e_result handleRequest();
	Connection::e_result sendResponse();

	bool hasPendingResponses() const;

	void resetForReuse();
	void clearRequest();

	e_status getStatus() const;
	e_status setStatus(Connection::e_status);

	void closeSocketFd();

	class GenericException : public std::exception {
	public:
		const char *what() const throw();
	};

	ConnWorker			   &operator=(const ConnWorker &);
	const Connection	   &getConn() const;
	const Connection	   *getConnPtr() const;
	CgiHandler::CGISession *getCgiSession() const;
	void					setCgiSession(CgiHandler::CGISession *session);
	void					clearCgiSession();

private:
	ConnectionContext _ctx;
};
#endif//WORKER_HPP
