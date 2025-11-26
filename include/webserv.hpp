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
#include <cerrno>
#include <cstring>
#include <map>
#include <string>
#include <netinet/in.h>
#include <vector>

#define FT_RED		"\e[31m"
#define FT_GREEN	"\e[32m"
#define FT_BLUE		"\e[33m"
#define FT_RESET	"\e[0m"

typedef std::map<const std::string, std::string> StringMap;

struct Config
{
	struct Http {
		struct Server {
			Server& operator=(const Server& other)
			{
				if (this == &other)
					return *this;
				ipv4_listen = other.ipv4_listen;
				server_name = other.server_name;
				location = other.location;
				return *this;
			}

			sockaddr_in	ipv4_listen;
			std::string			server_name;
			struct Location {
				std::string path;
				struct Conf {
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
				   sizeof(sockaddr_in)) != 0)
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
