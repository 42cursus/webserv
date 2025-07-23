/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:22:49 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 20:22:50 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include "Parser.hpp"

/*
struct Config
{
	struct {
		struct {
			struct sockaddr_in ipv4_listen;
			struct {
				char *path;
				struct {
					char *root;
					char **index;
				}	config;
			} location;
		} server;
	} http;
};
 */

Config Parser::make_default_config()
{
	static const char *index[] = {
		"index.html",
		"index.htm"
	};

	Config cfg = {
		.http = {
			.server = {
				.ipv4_listen = {
					.sin_family = AF_INET,
					.sin_port = htons(8080),
					.sin_addr = {
						.s_addr = htonl(INADDR_ANY)
					},
					.sin_zero = {0x00}
				},
				.location = {
					.path = (char *)"/",
					.config = {
						.root = (char *)"./resources/web",
						.index = (char **)index
					}
				}
			}
		}
	};
	return cfg;
}

Config Parser::parse(const char *filename)
{
	Config conf = make_default_config();

	std::ifstream file(filename);
	if (!file.is_open())
		std::cerr << "Err: File doesn't exist or can't be opened." << std::endl;
	else
	{
		std::string line;
		while (std::getline(file, line))
			std::cout << line << std::endl;
	}
	file.close();

	return (conf);
}