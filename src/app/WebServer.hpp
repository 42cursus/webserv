/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 20:59:25 by abelov            #+#    #+#             */
/*   Updated: 2026/01/31 20:59:25 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "WorkerPool.hpp"


#define TAG_MASK 0xF000000000000000
#define DETAG_MASK 0x00FFFFFFFFFFFFFF
#define TAG_A 0xA000000000000000
#define TAG_B 0xB000000000000000
#define TAG_C 0xC000000000000000

class WebServer {
public:
    enum epoll_ptr_type {
        EP_SRV,
        EP_WRKR,
        EP_CGIS,
        EP_NONE,
    };

    WebServer();
    ~WebServer();

    int init(std::vector<Config> &cfgs);
    int start();
    int serve();
    int stop();

private:
    static const unsigned int       WRKR_POOL_SIZE;
    static const unsigned int       EVS_SIZE;
    WorkerPool                      _wrkrPool;
    std::vector<TCPServer *>        _servers;
    std::vector<struct epoll_event> _events;
    int                             _epoll_fd;
    int epoll_mod(int fd, void *tagged_ptr, EPOLL_EVENTS events);
    int epoll_del(int fd);
    void serve_handle_worker(ConnWorker *wrkr, epoll_event &ev);
};

WebServer::epoll_ptr_type   get_tag(void *ptr);
void			            *tag_ptr(void *ptr, WebServer::epoll_ptr_type tag);
void        	            *detag_ptr(void *ptr);

#endif//WEBSERVER_HPP
