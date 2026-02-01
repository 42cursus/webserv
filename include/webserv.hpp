/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:27:09 by abelov            #+#    #+#             */
/*   Updated: 2026/02/01 17:52:23 by margo            ###   ########.fr       */
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
#include <algorithm>
#include "State.hpp"

#define FT_RED		"\e[31m"
#define FT_GREEN	"\e[32m"
#define FT_YELLOW	"\e[33m"
#define FT_BLUE		"\e[34m"
#define FT_MAGENTA	"\e[35m"
#define FT_CYAN		"\e[36m"
#define FT_BOLD		"\e[1m"
#define FT_RESET	"\e[m"

typedef std::map<const std::string, std::string> StringMap;

struct TrieNode;



struct Config
{
	struct Http {
		struct Server {
			sockaddr_in				ipv4_listen;
			std::string				server_name;
			std::vector<Location>	locations;
			TrieNode				*loc_trie;
			std::map<std::string, std::string>	error_pages; // map<error code, path to html>
		}	server;
	}	http;

};

std::string itoa(int);

#endif //WEBSERV_HPP
