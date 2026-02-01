/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 19:27:09 by abelov            #+#    #+#             */
/*   Updated: 2026/01/23 15:05:04 by fsmyth           ###   ########.fr       */
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

enum StatusCode
{
	SC_100 = 100,
	SC_101,
	SC_102,
	SC_103,
	SC_200 = 200,
	SC_201,
	SC_202,
	SC_203,
	SC_204,
	SC_205,
	SC_206,
	SC_207,
	SC_208,
	SC_226 = 226,
	SC_300 = 300,
	SC_301,
	SC_302,
	SC_303,
	SC_304,
	SC_307,
	SC_308,
	SC_400 = 400,
	SC_401,
	SC_402,
	SC_403,
	SC_404,
	SC_405,
	SC_406,
	SC_407,
	SC_408,
	SC_409,
	SC_410,
	SC_411,
	SC_412,
	SC_413,
	SC_414,
	SC_415,
	SC_416,
	SC_417,
	SC_418,
	SC_421 = 421,
	SC_422,
	SC_423,
	SC_424,
	SC_425,
	SC_426,
	SC_428,
	SC_429,
	SC_431 = 431,
	SC_451 = 451,
	SC_500 = 500,
	SC_501,
	SC_502,
	SC_503,
	SC_504,
	SC_505,
	SC_506,
	SC_507,
	SC_508,
	SC_510 = 510,
	SC_511,
	SC_MAX = 599
};

std::string itoa(int);

#endif //WEBSERV_HPP
