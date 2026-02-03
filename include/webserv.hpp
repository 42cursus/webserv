/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:27:09 by abelov            #+#    #+#             */
/*   Updated: 2026/02/03 00:31:55 by fsmyth           ###   ########.fr       */
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

#define FT_WHITE		"\e[39m"
#define FT_RED			"\e[31m"
#define FT_GREEN		"\e[32m"
#define FT_YELLOW		"\e[33m"
#define FT_BLUE			"\e[34m"
#define FT_MAGENTA		"\e[35m"
#define FT_CYAN			"\e[36m"
#define FT_BOLD			"\e[1m"

#define FT_WHITE_BG		"\e[48m"
#define FT_RED_BG		"\e[41m"
#define FT_GREEN_BG		"\e[42m"
#define FT_YELLOW_BG	"\e[43m"
#define FT_BLUE_BG		"\e[44m"
#define FT_MAGENTA_BG	"\e[45m"
#define FT_CYAN_BG		"\e[46m"

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
			std::vector<Redirect>	redirects;
			TrieNode				*loc_trie;
			TrieNode				*redirect_trie;
			std::map<std::string, std::string>	error_pages; // map<error code, path to html>
		}	server;
	}	http;

};

std::string itoa(int);

#endif //WEBSERV_HPP
