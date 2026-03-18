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
	ctx.conn.setParent(this);
}

ConnWorker::ConnWorker(const ConnWorker &other) : ctx(other.ctx) {
	ctx.conn.setParent(this);
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

void ConnWorker::refreshBackpressureState()
{
	ctx.conn.updateBackpressureState();
}

void ConnWorker::resetForReuse()
{
	ctx.resetForReuse();
}

void ConnWorker::clearRequest()
{
	ctx.conn.clearRequest();
}

void ConnWorker::closeSocketFd()
{
	ctx.conn.closeSocketFd();
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

void ConnWorker::setSrv(TCPServer* srv)
{
	ctx.conn.setSrv(srv);
}

void ConnWorker::setConnFd(int connFd)
{
	ctx.conn.setFd(connFd);
}

int ConnWorker::getConnFd() const
{
	return ctx.conn.getFd();
}

ConnWorker::e_status ConnWorker::getStatus() const
{
	return ctx.conn.getStatus();
}

ConnWorker::e_status ConnWorker::setStatus(Connection::e_status status)
{
	return ctx.conn.setStatus(status);
}

const Connection &ConnWorker::getConn() const {
	return ctx.conn;
}

const Connection *ConnWorker::getConnPtr() const {
	return &ctx.conn;
}

CgiHandler::CGISession *ConnWorker::getCgiSession() const
{
	return ctx.getCgiSession();
}

void ConnWorker::setCgiSession(CgiHandler::CGISession *session)
{
	ctx.setCgiSession(session);
}

void ConnWorker::clearCgiSession()
{
	ctx.clearCgiSession();
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
