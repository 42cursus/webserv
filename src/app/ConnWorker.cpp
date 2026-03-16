/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnWorker.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:02:20 by abelov            #+#    #+#             */
/*   Updated: 2026/01/30 04:19:00 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnWorker.hpp"
#include "Connection.hpp"
#include "Prefix_suffix.hpp"
#include <algorithm>

/*
** -------------------------------- STATIC VARS -------------------------------
*/

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

ConnWorker::ConnWorker()
{
	_ctx.bindOwner(this);
}

ConnWorker::ConnWorker(const ConnWorker &other) : _ctx(other._ctx) {
	_ctx.bindOwner(this);
}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/

ConnWorker::~ConnWorker()
{
    // Connection destructor closes fd; keep explicit close safe:
	closeSocketFd();
	resetForReuse();
}

/*
** -------------------------------- OPERATORS ---------------------------------
*/

ConnWorker &ConnWorker::operator=(const ConnWorker &other) {
    this->~ConnWorker();
    return *new(this) ConnWorker(other);
}

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

Connection::e_result ConnWorker::handleRequest()
{
	return _ctx.handleRequest();
}

Connection::e_result ConnWorker::sendResponse()
{
	return _ctx.sendResponse();
}

bool ConnWorker::hasPendingResponses() const
{
	return _ctx.conn.hasPendingResponses();
}

void ConnWorker::resetForReuse()
{
	_ctx.resetForReuse();
}

void ConnWorker::clearRequest()
{
	_ctx.clearRequest();
}

void ConnWorker::closeSocketFd()
{
	_ctx.closeSocketFd();
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

void ConnWorker::setSrv(TCPServer* srv)
{
	_ctx.setSrv(srv);
}

void ConnWorker::setConnFd(int connFd)
{
	_ctx.setConnFd(connFd);
}

int ConnWorker::getConnFd() const
{
	return _ctx.conn.getFd();
}

ConnWorker::e_status ConnWorker::getStatus() const
{
	return _ctx.conn.getStatus();
}

ConnWorker::e_status ConnWorker::setStatus(Connection::e_status status)
{
	return _ctx.conn.setStatus(status);
}

const Connection &ConnWorker::getConn() const {
	return _ctx.conn;
}

const Connection *ConnWorker::getConnPtr() const {
	ConnectionContext receiver = _ctx;
	return &receiver.conn;
}

CgiHandler::CGISession *ConnWorker::getCgiSession() const
{
	return _ctx.getCgiSession();
}

void ConnWorker::setCgiSession(CgiHandler::CGISession *session)
{
	_ctx.setCgiSession(session);
}

void ConnWorker::clearCgiSession()
{
	_ctx.clearCgiSession();
}

/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

const char *ConnWorker::GenericException::what() const throw()
{
    return "Client exception happened";
}

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/
