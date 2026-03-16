/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiSessionManager.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:49:27 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 16:49:27 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGISESSIONMANAGER_HPP
#define CGISESSIONMANAGER_HPP

#include "CgiHandler.hpp"
#include <sys/epoll.h>

class CgiSessionManager {
public:
	static void handleEvent(CgiHandler::CGISession *cgiSession, epoll_event &ev, int epoll_fd);

private:
	static int epollMod(int epoll_fd, int fd, void *tagged_ptr, uint32_t events);
	static int epollDel(int epoll_fd, int fd);
};

#endif//CGISESSIONMANAGER_HPP
