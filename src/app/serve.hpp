/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serve.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsmyth <fsmyth@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/22 14:50:26 by fsmyth            #+#    #+#             */
/*   Updated: 2026/01/22 15:08:39 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVE_HPP
# define SERVE_HPP

#include <vector>
#include "TCPServer.hpp"

enum epoll_ptr_type {
	EP_SRV,
	EP_WRKR,
	EP_NONE,
};

void			*tag_ptr(void *ptr, epoll_ptr_type tag);
epoll_ptr_type 	get_tag(void *ptr);
void        	*detag_ptr(void *ptr);
int 			serve(std::vector<TCPServer *> srvs);

#endif
