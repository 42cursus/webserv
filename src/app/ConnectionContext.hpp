/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionContext.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 17:39:49 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 17:39:49 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONCONTEXT_HPP
#define CONNECTIONCONTEXT_HPP

#include "CgiHandler.hpp"
#include "Connection.hpp"

class ConnWorker;

class ConnectionContext {
public:
	typedef Connection::e_status e_status;

	ConnectionContext();
	ConnectionContext(const ConnectionContext &other);
	~ConnectionContext();

	ConnectionContext &operator=(const ConnectionContext &other);

	void bindOwner(ConnWorker *owner);
	void setSrv(TCPServer *srv);
	void setConnFd(int connFd);

	Connection::e_result handleRequest();
	Connection::e_result sendResponse();
	void				 resetForReuse();
	void				 clearRequest();
	void				 closeSocketFd();

	CgiHandler::CGISession *getCgiSession() const;
	void					setCgiSession(CgiHandler::CGISession *session);
	void					clearCgiSession();

	Connection				conn;

private:
	CgiHandler::CGISession *_cgiSession;
};

#endif//CONNECTIONCONTEXT_HPP
