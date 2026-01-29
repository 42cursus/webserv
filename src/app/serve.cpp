/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serve.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsmyth <fsmyth@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 14:24:50 by fsmyth            #+#    #+#             */
/*   Updated: 2026/01/22 17:48:23 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include "ConnWorker.hpp"
#include "WorkerPool.hpp"
#include "serve.hpp"

#define TAG_MASK 0xF000000000000000
#define DETAG_MASK 0x00FFFFFFFFFFFFFF
#define TAG_A 0xA000000000000000
#define TAG_B 0xB000000000000000

void	*tag_ptr(void *ptr, epoll_ptr_type tag)
{
	uint64_t	tagged = reinterpret_cast<uint64_t>(ptr);

	// std::cout << std::hex << "orig:      " << reinterpret_cast<void*>(tagged)<< std::endl;
	tagged &= DETAG_MASK;
	// std::cout << std::hex << "after mask:" << reinterpret_cast<void*>(tagged)<< std::endl;

	switch (tag) {
		case (EP_SRV):
			tagged |= TAG_A;
			break;
		case (EP_WRKR):
			tagged |= TAG_B;
			break;
		case (EP_NONE):
			break;
	}
	// std::cout << std::hex << "after tag: " << reinterpret_cast<void*>(tagged)<< std::endl;
	return reinterpret_cast<void*>(tagged);
}

epoll_ptr_type	get_tag(void *ptr)
{
	uint64_t	tag = (reinterpret_cast<uint64_t>(ptr) & TAG_MASK) >> 60;

	switch (tag) {
		case (0xA):
			// std::cout << "ptr tagged as server" << std::endl;
			return EP_SRV;
		case (0xB):
			// std::cout << "ptr tagged as worker" << std::endl;
			return EP_WRKR;
		default:
			return EP_NONE;
	}
	// std::cout << "ptr tag failed" << std::endl;
}

void	*detag_ptr(void *ptr)
{
	uint64_t	tagged = reinterpret_cast<uint64_t>(ptr);

	return reinterpret_cast<void *>(tagged & DETAG_MASK);
}

void epoll_del(int epoll_fd, int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void epoll_mod(int epoll_fd, int fd, void* tagged_ptr, uint32_t events)
{
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.data.ptr = tagged_ptr;
    ev.events = events;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}


void serve_handle_worker(ConnWorker *wrkr, int epoll_fd, WorkerPool &wrkrPool, struct epoll_event &ev)
{
	if (ev.events & (EPOLLERR | EPOLLHUP))
	{
		std::cout << "error occured on fd: " << wrkr->getConnFd() << "(EPOLLERR | EPOLLHUP)" << std::endl;
        epoll_del(epoll_fd, wrkr->getConnFd());
        wrkr->resetForReuse();
		wrkrPool.free(wrkr);
	}
	else if (ev.events & EPOLLIN)
	{
		// std::cout << "Read ready on fd " << std::endl;
		int retval = wrkr->handleRequest();
        if (wrkr->getStatus() == Connection::REQ_RESPONSE_READY)
            epoll_mod(epoll_fd, wrkr->getConnFd(), tag_ptr(wrkr, EP_WRKR), EPOLLOUT);
        if (retval == 2)
		{
			std::cout << "Connection closed on fd: " << wrkr->getConnFd() << std::endl;
            epoll_del(epoll_fd, wrkr->getConnFd());
            wrkr->resetForReuse();
			wrkrPool.free(wrkr);
		}
	}
	else if (ev.events & EPOLLOUT && wrkr->getStatus() == Connection::REQ_RESPONSE_READY)
	{
		// std::cout << "Write ready on fd: " << wrkr->getConnFd() << std::endl;
		int retval = wrkr->sendResponse();
        if (retval == 0)
            epoll_mod(epoll_fd, wrkr->getConnFd(), tag_ptr(wrkr, EP_WRKR), EPOLLIN);
    }
}

int serve(std::vector<TCPServer *> srvs)
{
	extern sig_atomic_t				g_var;
	WorkerPool						wrkrPool(TCPServer::WRKR_POOL_SIZE);
	int								epoll_fd = epoll_create(1);
	std::vector<struct epoll_event>	evs;
	int								nfds;
    ConnWorker* 					wrkr;
	TCPServer*						srv;

	evs.resize(EVS_SIZE);
	for (uint64_t i = 0; i < srvs.size(); i++)
	{
		evs[i].events = EPOLLIN;
		evs[i].data.ptr = tag_ptr(srvs[i], EP_SRV);
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, srvs[i]->getSocketFd(), &evs[i]);
	}

	while(g_var != SIGINT)
	{
		nfds = epoll_wait(epoll_fd, evs.data(), 1024, -1);
		for (int i = 0; i < nfds; i++)
		{
			void *ptr = evs[i].data.ptr;
			switch (get_tag(ptr)) {
				case (EP_SRV):
					if (evs[i].events & EPOLLIN)
					{
						srv = reinterpret_cast<TCPServer*>(detag_ptr(ptr));
                        srv->acceptAllPendingConns(wrkrPool, epoll_fd);
					}
					break;
				case (EP_WRKR):
					wrkr = reinterpret_cast<ConnWorker *>(detag_ptr(ptr));
					serve_handle_worker(wrkr, epoll_fd, wrkrPool, evs[i]);
					break;
				default:
					break;
			}
		}
	}
	close(epoll_fd);
	return 0;

}
