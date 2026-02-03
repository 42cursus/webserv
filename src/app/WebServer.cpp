/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 21:42:24 by abelov            #+#    #+#             */
/*   Updated: 2026/02/03 00:57:28 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include "Logging.hpp"

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
{}

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

    std::vector<Config>::iterator cfg;
    for (cfg = cfgs.begin(); cfg != cfgs.end(); ++cfg) {
        Config &config = *cfg;
        TCPServer *server = new TCPServer(config);
        _servers.push_back(server);
    }

    return 0;
}

int WebServer::start()
{
    std::vector<TCPServer *>::iterator srv;
    for (srv = _servers.begin(); srv != _servers.end(); ++srv) {
        TCPServer *server = *srv;
        server->start();
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
    return 0;
}


void WebServer::serve_handle_worker(ConnWorker *wrkr, struct epoll_event &ev)
{
    const bool hup = (ev.events & EPOLLHUP);

    if (ev.events & EPOLLERR)
    {
        // std::cout << "error occured on fd: " << wrkr->getConnFd() << " (EPOLLERR)" << std::endl;
		log_connection(*wrkr, ERR);
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
			log_connection(*wrkr, DISCONNECT);
            epoll_del(wrkr->getConnFd());
            wrkr->resetForReuse();
            _wrkrPool.free(wrkr);
            return;
        }

        if (wrkr->getStatus() == Connection::HANDLING_CGI)
        {
            wrkr->cgiSession->register_write_pipe(_epoll_fd);
            wrkr->cgiSession->register_read_pipe(_epoll_fd);
            epoll_del(wrkr->getConnFd());
            //  FIXME: do stuff: too tired to figure out how to handle this rn
            return;
        }

        if (retval == Connection::WANT_WRITE || wrkr->getStatus() == Connection::READY_TO_WRITE)
            epoll_mod(wrkr->getConnFd(), tag_ptr(wrkr, WebServer::EP_WRKR), EPOLLOUT); // FIXME: now it can only be writte which is not true

        // If peer hung up and we have nothing queued to write, we can close now.
        if (hup && !wrkr->hasPendingResponses())
        {
            // std::cout << "Peer hung up (EPOLLHUP) and no pending responses on fd: " << wrkr->getConnFd() << std::endl;
			log_connection(*wrkr, HANGUP);
            epoll_del(wrkr->getConnFd());
            wrkr->resetForReuse();
            _wrkrPool.free(wrkr);
            return;
        }
    }
    // Checks if a WRITE can be performed without blocking
    else if (ev.events & EPOLLOUT && wrkr->getStatus() == Connection::READY_TO_WRITE)
    {
        // std::cout << "Write ready on fd: " << wrkr->getConnFd() << std::endl;
        Connection::e_result retval = wrkr->sendResponse();
        if (retval == Connection::CLOSED || retval == Connection::ERROR)
        {
            std::cout << "Connection closed on fd: " << wrkr->getConnFd() << std::endl;
			log_connection(*wrkr, DISCONNECT);
            epoll_del(wrkr->getConnFd());
            wrkr->resetForReuse();
            _wrkrPool.free(wrkr);
            return;
        }

        if (retval == Connection::OK)
        {
            // If peer hung up, don’t switch back to EPOLLIN; close once drained.
            if (hup && !wrkr->hasPendingResponses())
            {
                std::cout << "Finished writes after peer hangup on fd: " << wrkr->getConnFd() << std::endl;
                epoll_del(wrkr->getConnFd());
                wrkr->resetForReuse();
                _wrkrPool.free(wrkr);
                return;
            }
            epoll_mod(wrkr->getConnFd(), tag_ptr(wrkr, EP_WRKR), EPOLLIN);
        }
    }
}

int WebServer::serve()
{
    extern sig_atomic_t				g_var;
    ConnWorker* 					wrkr;
    CgiHandler*						cgiSession;

    while(g_var != SIGINT)
    {
        int nfds = epoll_wait(_epoll_fd, _events.data(), EVS_SIZE, -1);
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
                case (EP_CGIS): {
                    cgiSession = reinterpret_cast<CgiHandler *>(detag_ptr(ptr));
                    (void)cgiSession; // handle cgi IO
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

int WebServer::epoll_mod(int fd, void* tagged_ptr, EPOLL_EVENTS events)
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
        case (WebServer::EP_CGIS):
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
            return WebServer::EP_CGIS;
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
