/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:27:09 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 19:27:10 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <string>
#include <netinet/in.h>

struct Config
{
	struct {
		struct {
			struct sockaddr_in ipv4_listen;
			char	*server_name;
			struct {
				char	*path;
				struct {
					char	*root;
					char	**index;
				}	config;
			}	location;
		}	server;
	}	http;
};

#endif //WEBSERV_HPP
