/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiSessionManager.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:49:23 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 16:49:23 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiSessionManager.hpp"
#include "ConnWorker.hpp"
#include "Connection.hpp"
#include "HttpResponse.hpp"
#include "WebServer.hpp"
#include <csignal>
#include <cstring>

/*
** -------------------------------- STATIC VARS -------------------------------
*/

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/


/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

int CgiSessionManager::epollMod(int epoll_fd, int fd, void *tagged_ptr, uint32_t events)
{
	struct epoll_event ev = {};
	std::memset(&ev, 0, sizeof(ev));
	ev.data.ptr = tagged_ptr;
	ev.events = events;
	return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

int CgiSessionManager::epollDel(int epoll_fd, int fd)
{
	return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void CgiSessionManager::handleEvent(CgiHandler::CGISession *cgiSession, epoll_event &ev, int epoll_fd)
{
	if (cgiSession == NULL)
		return;

	if (ev.events & EPOLLIN) {
		Connection::e_result result = cgiSession->onReadable();
		if (result == Connection::OK) {
			epollDel(epoll_fd, cgiSession->_stdout_pipe[0]);
		} else if (result == Connection::WANT_WRITE) {
			ConnWorker *worker = cgiSession->_parentConnection->getParent();
			void	   *taggedPtr = tag_ptr(worker, WebServer::EP_WRKR);
			epollMod(epoll_fd, cgiSession->_parentConnection->getFd(), taggedPtr, EPOLLIN | EPOLLOUT);
		} else if (result == Connection::ERROR) {
			epollDel(epoll_fd, cgiSession->_stdout_pipe[0]);
			epollDel(epoll_fd, cgiSession->_stdin_pipe[1]);
			kill(cgiSession->_pid, SIGTERM);
			ConnWorker *worker = cgiSession->_parentConnection->getParent();
			if (worker != NULL)
				worker->clearCgiSession();
			delete cgiSession;
		}
		return;
	}

	if (ev.events & EPOLLOUT) {
		Connection::e_result result = cgiSession->onWritable();
		if (result == Connection::OK)
			epollDel(epoll_fd, cgiSession->_stdin_pipe[1]);
		return;
	}

	if (ev.events & (EPOLLERR | EPOLLHUP)) {
		HttpResponse *res = cgiSession->_parentConnection->getCurrentResponse();
		if (res != NULL)
			res->body_complete = true;
		epollDel(epoll_fd, cgiSession->_stdout_pipe[0]);
		epollDel(epoll_fd, cgiSession->_stdin_pipe[1]);
		ConnWorker *worker = cgiSession->_parentConnection->getParent();
		if (worker != NULL)
			worker->clearCgiSession();
		delete cgiSession;
	}
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/
