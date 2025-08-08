/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:22:49 by abelov            #+#    #+#             */
/*   Updated: 2025/08/08 16:39:59 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <sstream>
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

		TO DO:
			//1. open and read config file (copy in buf stream and close)
			2. read http 
			3. read server 
			4. read listening port > hostname/IP and TCP port
			5. read config stuff (root +  index + anything else)
			6. construct config struct
 */

Parser::Parser(IState*	currentState): _currentState(currentState), _key(""), _inBlock(false) {};

Parser::~Parser() 
{
	delete _currentState;
}

IState*	Parser::getCurrentState() const
{
	return _currentState;
}

void	Parser::setCurrentState(IState&	newState)
{
	_currentState = &newState;
}

bool	Parser::isInBlock() const
{
	return _inBlock;
}

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

std::string Parser::read_file(const char *filename)
{
	//Config conf = make_default_config();

	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Err: File doesn't exist or can't be opened." << std::endl;
		return std::string();
	}
	
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return buffer.str();
}

std::map<std::string, std::string> Parser::init_mime_types()
{
	std::ifstream file("./mime.types");
	
	if (!file.is_open())
	{
		std::cerr << "Err: mime.types can't be opened or doesn't exist." << std::endl;
		return std::map<std::string, std::string>();
	}
	
	std::string line;
	while (std::getline(file, line))
	{
		std::string	mtype, ext;
		if (line.find(';') == std::string::npos)
			continue ; // skips lines without semicolons
		size_t start = line.find_first_not_of(" \t"); // checks for whitespace/indentation
		size_t	end;
		if (start != 0)
		{	
			// extract substr and store them in map
			std::istringstream	iss(line);
			std::string	word;
			while (iss >> word)
			{
				end = word.length();
				mtype = line.substr(start, end);
				std::cout << mtype << std::endl;
			}	
		}
	}
	return (std::map<std::string, std::string>());
}
/*
Config Parser::parse(const char *filename)
{
	Config conf = make_default_config();
	std::stringstream raw_config = read_file(filename);
	std::string line;
	
	while (std::getline(raw_config, line))
	{
		if (line.empty()) // skips empty lines
			continue ;
		if (line.find('#') != line.npos)
			// ignore comments (everything after a #)
		;
		if (line.find("http"))
		// handle http
		;
		if (line.find("server"))
			// handle server
		;
		if (line.find("location"))
			// handle location
		;
		
	}
	// look for keywords for each section of config
	// http, server, location, 
	
	return (conf);
}
*/

int	main(int argc, char *argv[])
{
	Parser*	parse = new Parser();
	std::string	conf;
	char*	filename;

	(void)argc;
	filename = argv[1];
	conf = parse->read_file(filename);
	std::cout << conf << std::endl;
	std::map<std::string, std::string> mtypes = parse->init_mime_types();
}