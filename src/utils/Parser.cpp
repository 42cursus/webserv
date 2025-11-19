/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:22:49 by abelov            #+#    #+#             */
/*   Updated: 2025/10/01 23:51:47 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <sstream>
#include "Parser.hpp"
#include "Utils.hpp"

/*
** -------------------------------- STATIC VARS -------------------------------
*/


/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

Parser::Parser(IState*	currentState)
	: _currentState(currentState), _inBlock(false), _config()
{
	memSet(&_config, 0, sizeof(_config));
}

Parser::Parser()
{
	memSet(&_config, 0, sizeof(_config));
	_config = make_default_config();

}

Parser::Parser(const Parser &copy)
{
	(void)copy;
}

Parser::errorException::errorException(const std::string& msg): _errorMsg(msg) {};

const char* Parser::errorException::what() const throw()
{
	return _errorMsg.c_str();
}
/*
** ------------------------------- DESTRUCTOR ---------------------------------
*/

Parser::~Parser()
{
	delete _currentState;

	for (std::vector<IBlock*>::iterator it = _blocks.begin(); it != _blocks.end(); ++it)
		delete *it;
	_blocks.clear();
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
	const std::string index[] = {
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
//						.s_addr = inet_addr("127.0.0.1")
					},
					.sin_zero = {0x00}
				},
				.server_name = "localhost",
				.location = {
					.path = "/",
					.config = {
						.autoindex = true,
						.root = "./resources/web",
						.index = Utils::to_vector(index)
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

		TO DO:
			//1. open and read config file (copy in buf stream and close)
			2. read http 
			//3. read server
			//4. read listening port > hostname/IP and TCP port
			5. read config stuff (root +  index + anything else)
			6. construct config struct
 */


std::string Parser::getKey() const
{
	return _key;
}

void	Parser::setKey(std::string key)
{
	_key = key;
}

IState*	Parser::getCurrentState() const
{
	return _currentState;
}

void	Parser::setCurrentState(IState*	newState)
{
	_currentState = newState;
}

bool	Parser::isInBlock() const
{
	return _inBlock;
}

void	Parser::setInBlock(bool in)
{
	_inBlock = in;
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

void	Parser::toggleCurrentToken()
{
	_currentIt++;
	_currentToken = _nextToken;
	_nextToken = *_currentIt;
}

void	Parser::addNewBlock(IBlock* newBlock)
{
	_blocks.push_back(newBlock);
}

IBlock*	Parser::getBlock(std::string key)
{
	for (std::vector<IBlock*>::iterator it = _blocks.begin(); it != _blocks.end(); ++it)
	{
		if ((*it)->getName() == key)
			return *it;
	}

	return NULL;
}

t_token	Parser::makeToken(e_token	key, std::string word, int linecount)
{
	t_token	token;

	token.type = key;
	token.literal = word;
	token.line = linecount;
	
	return token;
}

Comment	Parser::makeComment(std::string buf, int line)
{
	Comment	comment;

	comment.content = buf;
	comment.rlidx = line;

	return comment;
}

void	Parser::toggle()
{
	_currentState->toggle(this);
}

std::vector<t_token>	Parser::tokenize()
{
	int	linecount = 0;
	std::string	line;
	std::vector<t_token>	tokens;
	
	std::ifstream file(_configRoot.c_str());
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
				tokens.push_back(makeToken(COMMENT, word, linecount)); // to do: function for storing comments
				break ;
			}
			if (word == "http" || word == "server" || word == "listen" || word == "location" || word == "root" || word == "index")
				tokens.push_back(makeToken(KEY, word, linecount));
			else if (word == "{")
				tokens.push_back(makeToken(BLOCK_START, word, linecount));
			else if (word == "}")
				tokens.push_back(makeToken(BLOCK_END, word, linecount));
			else if (word.find('/') != std::string::npos)
				tokens.push_back(makeToken(REGEX, word, linecount));
			else if (word[0] == '"' && word[word.length() - 1] == '"')
				tokens.push_back(makeToken(QUOTES, readQuotedString(word), linecount));
			else if (word[0] == '$')
				tokens.push_back(makeToken(VAR, word, linecount)); // to do: function extracting/expanding variable ???
			else if (word == ";" || word[word.length() - 1] == ';')
			{
				if (word != ";")
					tokens.push_back(makeToken(KEY, word.substr(0, word.length() - 1), linecount));
				tokens.push_back(makeToken(SEMICOLON, ";", linecount));
			}
			else if (word[word.length() - 1] == '\n' || word == "\n")
			{
				if (word != "\n")
					tokens.push_back(makeToken(KEY, word.substr(0, word.length() - 1), linecount));
				tokens.push_back(makeToken(EOL, "\n", linecount));
			}
			else
				tokens.push_back(makeToken(ILLEGAL, word, linecount));
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
		std::string	 mtype;
		if (line.find(';') == std::string::npos)
			continue ; // skips lines without semicolons
		size_t start = line.find_first_not_of(" \t"); // checks for whitespace/indentation
		size_t	end = 0;
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

bool	t_token::operator==(const t_token&	other) const
{
	return (type == other.type && literal == other.literal && line == other.line);
}

void	printTokens(std::vector<t_token> tokens)
{
	for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
	{
		if (it->type == EOL)
			std::cout << "EOL " << it->literal << " on line " << it->line << std::endl;
		else if (it->type == KEY)
			std::cout << "KEYWORD " << it->literal << " on line " << it->line << std::endl;
		else if (it->type == VAR)
			std::cout << "VARIABLE " << it->literal << " on line " << it->line << std::endl;	
		else if (it->type == QUOTES)
			std::cout << "QUOTED STRING " << it->literal << " on line " << it->line << std::endl;
		else if (it->type == BLOCK_START)
			std::cout << "BLOCK START " << it->literal << " on line " << it->line << std::endl;
		else if (it->type == BLOCK_END)
			std::cout << "BLOCK END " << it->literal << " on line " << it->line << std::endl;
		else if (it->type == SEMICOLON)
			std::cout << "SEMICOLON " << it->literal << " on line " << it->line << std::endl;
		else if (it->type == REGEX)
			std::cout << "REGEX " << it->literal << " on line " << it->line << std::endl;
		else if (it->type == COMMENT)
			std::cout << "COMMENT " << it->literal << " on line " << it->line << std::endl;
		else if (it->type == ILLEGAL)
			std::cout << "ILLEGAL " << it->literal << " on line " << it->line << std::endl;
		else
			std::cout << "UNKNOWN " << it->literal << " on line " << it->line << std::endl;
	}
}

void	Parser::parseServer(std::vector<t_token>& tokens)
{
	std::vector<t_token>::iterator it;
	
	_config.http.server.ipv4_listen.sin_family = AF_INET;
	_config.http.server.ipv4_listen.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(_config.http.server.ipv4_listen.sin_zero, 0, sizeof(_config.http.server.ipv4_listen.sin_zero));
	for(it = _currentIt; it != tokens.end(); ++it)
	{
		if (it->literal == "listen" && (it + 1) != tokens.end())
		{
			int port;
			std::stringstream ss((it + 1)->literal);
			ss >> port;
			if (port < 1 || port > 65636)
				throw errorException("Invalid config: port");
			_config.http.server.ipv4_listen.sin_port = htons(port);
			++it;
		}
		if (it->literal == "server_name" && (it + 1) != tokens.end())
		{
			_config.http.server.server_name = (it + 1)->literal;
			//std::cout << _config.http.server.server_name << std::endl;
			++it;
		}
	}
}

void Parser::parse(std::vector<t_token>&	tokens)
{
	std::vector<t_token>::iterator it;
	
	setCurrentState(new Start());
	for (it = tokens.begin(); it != tokens.end(); ++it)
	{
		_currentIt = it;
		_currentToken = *it;
		if (it + 1 != tokens.end())
			_nextToken = *(it + 1);
		
		//std::cout << getCurrentToken().literal << ", " << getCurrentToken().line << std::endl;
		toggle();
		if (dynamic_cast<Server*>(_currentState) != NULL)
		{
			// populate Config struct w sockaddr_in and server_name
			parseServer(tokens);
			it = _currentIt;
		}
		else if (dynamic_cast<Location*>(_currentState) != NULL)
		{
			// populate Config struct w path
			toggle();
			it = _currentIt;
		}
		/*
		else if (dynamic_cast<locConfig*>(_currentState) != NULL)
		{
			// populate Config struct w root and index
		}
		*/
	}
	
}

bool Parser::operator==(const Parser &rhs) const
{
	return _key == rhs._key &&
		   _configRoot == rhs._configRoot &&
		   _currentToken == rhs._currentToken &&
		   _nextToken == rhs._nextToken &&
		   _currentIt == rhs._currentIt &&
		   _currentState == rhs._currentState &&
		   _inBlock == rhs._inBlock &&
		   _blocks == rhs._blocks &&
		   commentBuf == rhs.commentBuf &&
		   _config == rhs._config;
}

bool Parser::operator!=(const Parser &rhs) const
{
	return !(rhs == *this);
}

void	printConfig(Config cfg)
{
	int family;
	uint16_t port;
	uint32_t addr;
	std::string locationPath;
	std::string configRoot;
	std::string server_name;

	family = cfg.http.server.ipv4_listen.sin_family;
	port = ntohs(cfg.http.server.ipv4_listen.sin_port);
	addr = ntohl(cfg.http.server.ipv4_listen.sin_addr.s_addr);
	server_name = cfg.http.server.server_name;
	locationPath = cfg.http.server.location.path;
	configRoot = cfg.http.server.location.config.root;

	std::cout << "Address Family: " << family << std::endl
              << "Port: " << port << std::endl
              << "IP Address: " << ((addr >> 24) & 0xFF) << "."
                               << ((addr >> 16) & 0xFF) << "."
                               << ((addr >> 8) & 0xFF) << "."
                               << (addr & 0xFF) << std::endl
              << "Server Name: " << (server_name.empty() ? server_name : "NULL") << std::endl
			  << "Location Path: " << locationPath << std::endl
			  << "Config Root: " << configRoot << std::endl
			  << "Index Files: ";

	for (int i = 0; !cfg.http.server.location.config.index.empty(); i++)
	{
		std::cout << cfg.http.server.location.config.index[i];
		if (!cfg.http.server.location.config.index[i + 1].empty())
			std::cout << ", ";
	}
	std::cout << std::endl;
}

/*
	TO DO:
		1. finish parsing function
		//2. function to print Config struct for testing purposes
		3. update Config to be able to handle multiple servers
*/

/*
int	main(int argc, char *argv[])
{
	Parser*	parse = new Parser();
	std::vector<t_token> tokens;

	(void)argc;
	parse->setConfigRoot(argv[1]);
	tokens = parse->tokenize();
	//printTokens(tokens);
	parse->parse(tokens);
	printConfig(parse->_config);
}
*/
