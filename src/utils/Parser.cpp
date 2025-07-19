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


Config Parser::make_default_config()
{
	Config cfg = {0x00};
	std::memset(&cfg.server_addr, 0, sizeof(cfg.server_addr));
	cfg.server_addr.sin_family = AF_INET;
	cfg.server_addr.sin_port = htons(8080);
	cfg.server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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