/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 21:42:24 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 17:44:19 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include "CgiSessionManager.hpp"
#include "HttpResponse.hpp"
#include "Logging.hpp"
#include "WorkerPool.hpp"
#include <csignal>
#include <exception>
#include <sys/epoll.h>

/*
** -------------------------------- STATIC VARS -------------------------------
*/

const unsigned int WebServer::WRKR_POOL_SIZE = 1024;
const unsigned int WebServer::EVS_SIZE = 1024;

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

WebServer::WebServer() :
    _wrkrPool(WRKR_POOL_SIZE),
    _events(EVS_SIZE)
{}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/

WebServer::~WebServer()
{
}

/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

int WebServer::init(std::vector<Config>	&cfgs)
{
    // man 2 epoll_create: size must be greater than zero to ensure backward compatibility
    this->_epoll_fd = epoll_create(1);
	int	srv_count = 0;

    std::vector<Config>::iterator cfg;
    for (cfg = cfgs.begin(); cfg != cfgs.end(); ++cfg) {
        Config &config = *cfg;
        TCPServer *server = new TCPServer(config);
		server->idx = srv_count++;
        _servers.push_back(server);
    }

    return 0;
}

int WebServer::start()
{
    std::vector<TCPServer *>::iterator srv;
    std::vector<TCPServer *> started;
    for (srv = _servers.begin(); srv != _servers.end(); ++srv) {
        TCPServer *server = *srv;
		try {
			server->start();
			started.push_back(server);
		} catch (std::exception &e) {
			server->stop();
			delete server;
		}
    }
	_servers = started;

	if (_servers.size() == 0)
	{
		log_no_servers();
		return 1;
	}

    for (uint64_t i = 0; i < _servers.size(); i++)
    {
        _events[i].events = EPOLLIN;
        _events[i].data.ptr = tag_ptr(_servers[i], EP_SRV);
        epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, _servers[i]->getSocketFd(), &_events[i]);
    }
    return 0;
}

int WebServer::stop()
{
    for (uint64_t i = 0; i < _servers.size(); i++)
    {
        _servers[i]->stop();
        delete _servers[i];
    }
	_wrkrPool.killOrphans();
	log_shutdown();
    return 0;
}


void WebServer::serve_handle_worker(ConnWorker *wrkr, struct epoll_event &ev)
{
    const bool hup = (ev.events & EPOLLHUP);

    if (ev.events & EPOLLERR)
    {
        // std::cout << "error occured on fd: " << wrkr->getConnFd() << " (EPOLLERR)" << std::endl;
		log_connection(*wrkr, CONN_ERROR);
        epoll_del(wrkr->getConnFd());
        wrkr->resetForReuse();
        _wrkrPool.free(wrkr);
        return;
    }

    // Checks if a READ can be performed without blocking
    if (ev.events & EPOLLIN)
    {
        // std::cout << "Read ready on fd" << std::endl;
        Connection::e_result retval = wrkr->handleRequest();
        if (retval == Connection::CLOSED || retval == Connection::ERROR)
        {
            // std::cout << "Connection closed on fd: " << wrkr->getConnFd() << std::endl;
			log_connection(*wrkr, CONN_DISCONNECT);
            epoll_del(wrkr->getConnFd());
            wrkr->resetForReuse();
            _wrkrPool.free(wrkr);
            return;
        }

        if (wrkr->getStatus() == Connection::HANDLING_CGI)
        {
            CgiHandler::CGISession *cgiSession = wrkr->getCgiSession();
            if (cgiSession == NULL)
                return;
            cgiSession->register_write_pipe(_epoll_fd);
            cgiSession->register_read_pipe(_epoll_fd);

            // epollDel(wrkr->getConnFd());
        	wrkr->setStatus(Connection::READY_TO_WRITE);

            return;
        }

        // If peer hung up, and we have nothing queued to write, we can close now.
        if (hup && !wrkr->hasPendingResponses())
        {
            // std::cout << "Peer hung up (EPOLLHUP) and no pending responses on fd: " << wrkr->getConnFd() << std::endl;
			log_connection(*wrkr, CONN_HANGUP);
            epoll_del(wrkr->getConnFd());
            wrkr->resetForReuse();
            _wrkrPool.free(wrkr);
            return;
        }
    }
    // Checks if a WRITE can be performed without blocking
    if (ev.events & EPOLLOUT && wrkr->getStatus() == Connection::READY_TO_WRITE)
    {
        // std::cout << "Write ready on fd: " << wrkr->getConnFd() << std::endl;
        Connection::e_result retval = wrkr->sendResponse();
        if (retval == Connection::CLOSED || retval == Connection::ERROR)
        {
            // std::cout << "Connection closed on fd: " << wrkr->getConnFd() << std::endl;
			log_connection(*wrkr, CONN_DISCONNECT);
            epoll_del(wrkr->getConnFd());
            wrkr->resetForReuse();
            _wrkrPool.free(wrkr);
            return;
        }

        if (retval == Connection::OK)
        {
            // If peer hung up, don’t switch back to EPOLLIN; close once drained.
        	// std::cout << "Returned Connection::OK" << std::endl;
            if (hup && !wrkr->hasPendingResponses())
            {
                std::cout << "Finished writes after peer hangup on fd: " << wrkr->getConnFd() << std::endl;
                epoll_del(wrkr->getConnFd());
                wrkr->resetForReuse();
                _wrkrPool.free(wrkr);
                return;
            }
        }
    }

	wrkr->refreshBackpressureState();
	uint32_t events = 0;
	if (wrkr->shouldReadFromSocket())
		events |= EPOLLIN;
	if (wrkr->getStatus() == Connection::READY_TO_WRITE)
		events |= EPOLLOUT;
	if (events == 0)
		events = EPOLLOUT;
	epoll_mod(wrkr->getConnFd(), tag_ptr(wrkr, EP_WRKR), events);
}

int WebServer::serve()
{
    extern sig_atomic_t				g_var;
    ConnWorker* 					wrkr;

	while(g_var != SIGINT)
    {
        int nfds = epoll_wait(_epoll_fd, &_events[0], EVS_SIZE, -1); // event demultiplexer
        for (int i = 0; i < nfds; i++)
        {
            void *ptr = _events[i].data.ptr;
            switch (get_tag(ptr)) {
                case (EP_SRV): {
                    if (_events[i].events & EPOLLIN)
                    {
                        TCPServer* srv = reinterpret_cast<TCPServer*>(detag_ptr(ptr));
                        srv->acceptAllPendingConns(_wrkrPool, _epoll_fd);
                    }
                    break;
                }
                case (EP_WRKR): {
                    wrkr = reinterpret_cast<ConnWorker *>(detag_ptr(ptr));
                    serve_handle_worker(wrkr, _events[i]);
                    break;
                }
                case (EP_CGI): {
                    CgiHandler::CGISession* cgiSession = reinterpret_cast<CgiHandler::CGISession *>(detag_ptr(ptr));
                    CgiSessionManager::handleEvent(cgiSession, _events[i], _epoll_fd);
                    break;
                }
                default:
                    break;
            }
        }
    }

    close(_epoll_fd);
    return 0;
}

int WebServer::epoll_mod(int fd, void *tagged_ptr, uint32_t events)
{
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.data.ptr = tagged_ptr;
    ev.events = events;
    return  epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

int WebServer::epoll_del(int fd)
{
    return epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
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

void	*tag_ptr(void *ptr, WebServer::epoll_ptr_type tag)
{
    uint64_t	tagged = reinterpret_cast<uint64_t>(ptr);

    // std::cout << std::hex << "orig:      " << reinterpret_cast<void*>(tagged)<< std::endl;
    tagged &= DETAG_MASK;
    // std::cout << std::hex << "after mask:" << reinterpret_cast<void*>(tagged)<< std::endl;

    switch (tag) {
        case (WebServer::EP_SRV):
            tagged |= TAG_A;
            break;
        case (WebServer::EP_WRKR):
            tagged |= TAG_B;
            break;
        case (WebServer::EP_CGI):
            tagged |= TAG_C;
            break;
        case (WebServer::EP_NONE):
            break;
    }
    // std::cout << std::hex << "after tag: " << reinterpret_cast<void*>(tagged)<< std::endl;
    return reinterpret_cast<void*>(tagged);
}

WebServer::epoll_ptr_type	get_tag(void *ptr)
{
    uint64_t	tag = (reinterpret_cast<uint64_t>(ptr) & TAG_MASK) >> 60;

    switch (tag) {
        case (0xA):
            // std::cout << "ptr tagged as server" << std::endl;
            return WebServer::EP_SRV;
        case (0xB):
            // std::cout << "ptr tagged as worker" << std::endl;
            return WebServer::EP_WRKR;
        case (0xC):
            // std::cout << "ptr tagged as server" << std::endl;
            return WebServer::EP_CGI;
        default:
            return WebServer::EP_NONE;
    }
    // std::cout << "ptr tag failed" << std::endl;
}

void	*detag_ptr(void *ptr)
{
    uint64_t	tagged = reinterpret_cast<uint64_t>(ptr);
    return reinterpret_cast<void *>(tagged & DETAG_MASK);
}
