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
	_conn.setParent(this);
}

ConnWorker::ConnWorker(const ConnWorker &other) : _conn(other._conn) {
	_conn.setParent(this);
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
    return _conn.onReadable();
}

Connection::e_result ConnWorker::sendResponse()
{
	return _conn.onWritable();
}

bool ConnWorker::hasPendingResponses() const
{
    return _conn.hasPendingResponses();
}

void ConnWorker::resetForReuse()
{
    _conn.reset();
}

void ConnWorker::clearRequest()
{
    _conn.clearRequest();
}

void ConnWorker::closeSocketFd()
{
    _conn.closeSocketFd();
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

void ConnWorker::setSrv(TCPServer* srv)
{
    _conn.setSrv(srv);
}

void ConnWorker::setConnFd(int connFd)
{
    _conn.setFd(connFd);
}

int ConnWorker::getConnFd() const
{
    return _conn.getFd();
}

ConnWorker::e_status ConnWorker::getStatus() const
{
    return _conn.getStatus();
}

ConnWorker::e_status ConnWorker::setStatus(Connection::e_status status)
{
    return _conn.setStatus(status);
}

const Connection &ConnWorker::getConn() const {
    return _conn;
}

const Connection *ConnWorker::getConnPtr() const {
    return &_conn;
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
