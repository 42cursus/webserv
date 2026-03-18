/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionContext.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 17:39:41 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 17:39:41 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionContext.hpp"
#include "ConnWorker.hpp"

/*
** -------------------------------- STATIC VARS -------------------------------
*/

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

ConnectionContext::ConnectionContext() : conn(), _cgiSession(NULL)
{}

ConnectionContext::ConnectionContext(const ConnectionContext &other) :
	conn(other.conn), _cgiSession(NULL)
{}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/

ConnectionContext::~ConnectionContext()
{
	conn.closeSocketFd();
	resetForReuse();
}


/*
** -------------------------------- OPERATORS ---------------------------------
*/

ConnectionContext &ConnectionContext::operator=(const ConnectionContext &other)
{
	if (this != &other) {
		this->~ConnectionContext();
		return *new(this) ConnectionContext(other);
	}
	return *this;
}


/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

Connection::e_result ConnectionContext::handleRequest()
{
	return conn.onReadable();
}

Connection::e_result ConnectionContext::sendResponse()
{
	return conn.onWritable();
}

void ConnectionContext::resetForReuse()
{
	conn.reset();
	_cgiSession = NULL;
}

CgiHandler::CGISession *ConnectionContext::getCgiSession() const
{
	return _cgiSession;
}

void ConnectionContext::setCgiSession(CgiHandler::CGISession *session)
{
	_cgiSession = session;
}

void ConnectionContext::clearCgiSession()
{
	_cgiSession = NULL;
}


/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/













