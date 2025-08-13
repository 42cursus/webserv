/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:22:49 by abelov            #+#    #+#             */
/*   Updated: 2025/08/12 22:23:38 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <sstream>
#include "Parser.hpp"

/*
** -------------------------------- STATIC VARS -------------------------------
*/


/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

Parser::Parser(IState*	currentState)
	: _currentState(currentState),
	  _inBlock(false)
{

}

Parser::Parser()
{

}

Parser::Parser(const Parser &copy)
{
	(void)copy;
}

/*
** ------------------------------- DESTRUCTOR ---------------------------------
*/

Parser::~Parser()
{
	delete _currentState;
}


/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

std::string	Parser::readQuotedString(std::string word)
{
	if (word.length() >= 2 && word[0] == '"' && word[word.size() - 1] == '"')
		return word.substr(1, word.length() - 2);

	return word;
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
				.server_name = (char *)"localhost",
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

/*
** -------------------------------- ACCESSORS ---------------------------------
*/


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

std::string	Parser::getConfigRoot() const
{
	return _configRoot;
}

void	Parser::setConfigRoot(std::string configRoot)
{
	_configRoot = configRoot;
}

Config	Parser::getConfig() const
{
	return _config;
}

t_token	Parser::getCurrentToken() const
{
	return _currentToken;
}

void	Parser::setCurrentToken(t_token token)
{
	_currentToken = token;
}

t_token	Parser::getNextToken() const
{
	return _nextToken;
}

void	Parser::setNextToken(t_token token)
{
	_nextToken = token;
}

void	Parser::toggle()
{
	_currentState->toggle(this);
}

Config Parser::parse(const char *filename)
{
	return Config();
	(void)filename;
}

/*
std::vector<t_token>	Parser::tokenize()
{
	int	linecount = 0;
	std::string	line;
	std::vector<t_token>	tokens;
	
	std::ifstream file(_configRoot);
	if (!file.is_open())
	{
		std::cerr << "Err: File doesn't exist or can't be opened." << std::endl;
		return std::vector<t_token>();
	}
	
	while (std::getline(file, line))
	{
		linecount++;
		std::istringstream	iss(line);
		std::string	word;
		
		if (line.empty())
			continue ;

		while (iss >> word)
		{
			if (word[0] == '#')
			{
				tokens.push_back(t_token{COMMENT, word, linecount}); // to do: function for storing comments
				break ;
			}
			else if (word == "http" || word == "server" || word == "listen" || word == "location" || word == "root" || word == "index")
				tokens.push_back(t_token{KEY, word, linecount});
			else if (word == "{")
				tokens.push_back(t_token{BLOCK_START, word, linecount});
			else if (word == "}")
				tokens.push_back(t_token{BLOCK_END, word, linecount});
			else if (word.find('\\') != std::string::npos)
				tokens.push_back(t_token{REGEX, word, linecount});
			else if (word[0] == '"' && word.back() == '"')
				tokens.push_back(t_token{QUOTES, readQuotedString(word), linecount});
			else if (word[0] == '$')
				tokens.push_back(t_token{VAR, word, linecount}); // to do: function extracting/expanding variable ???
			else if (word == ";" || word.back() == ';')
			{
				if (word != ";")
					tokens.push_back(t_token{KEY, word.substr(0, word.length() - 1), linecount});
				tokens.push_back(t_token{SEMICOLON, ";", linecount});
			}
			else
				tokens.push_back(t_token{ILLEGAL, word, linecount});
		}
	}
	
	file.close();
	return tokens;
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
*/

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

/*
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
*/