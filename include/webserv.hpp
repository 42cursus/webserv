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
#include <vector>

struct Config
{
	struct {
		struct {
			struct sockaddr_in	ipv4_listen;
			std::string			server_name;
			struct {
				std::string path;
				struct {
					bool autoindex;
					std::string root;
					std::vector<std::string> index;
				}	config;
			}	location; // https://nginx.org/en/docs/http/ngx_http_core_module.html#location
		}	server;
	}	http;

	bool operator==(const Config &other) const
	{
		bool	ret = true;

		if (memcmp(&http.server.ipv4_listen, &other.http.server.ipv4_listen,
				   sizeof(struct sockaddr_in)) != 0)
			ret = false;
		if (http.server.server_name != other.http.server.server_name)
			ret = false;
		if (http.server.location.path != other.http.server.location.path)
			ret = false;
		if (http.server.location.config.root != other.http.server.location.config.root)
			ret = false;
		if (http.server.location.config.index != other.http.server.location.config.index)
			ret = false;
		return ret;
	}

	bool operator!=(const Config &other) const
	{
		return !(*this == other);
	}
};

#endif //WEBSERV_HPP
