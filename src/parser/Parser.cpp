/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 21:52:43 by margo             #+#    #+#             */
/*   Updated: 2026/03/16 15:34:07 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "ConfigLexer.hpp"
#include "ConfigValidator.hpp"
#include "State.hpp"

/*
    //- add handling for cgi_param > include  in CGI struct
    - check if vlaue in cgi_param is a var starting w $
    - check if method valid: PUT GET DELETE POST;
    - check if line type is always validated correctly;
    - check if line size is validated correctly;
    - check if last token before SEMICOLON on error_page line is .html file;
    - check if error_page code is valid;
    - check if bools are only true and false;
    - check if CGI script has correct extension;
*/


/*
** -------------------------------- STATIC VARS -------------------------------
*/

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

Parser::Parser(std::string filePath) : _config_root(filePath)
{}

Parser::~Parser()
{}

Parser::Error::Error(std::string msg) { _msg = msg; }

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/

Parser::Error::~Error() throw() {}

/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

void Parser::init_key_database()
{
	ConfigLexer::loadKeyDatabase(_key_database);
}

void Parser::init_directive_handlers()
{
	_directive_handlers["worker_processes"] = &Parser::handleWorkers;
	_directive_handlers["log_format"]		= &Parser::handleLogFormat;
	_directive_handlers["listen"]			= &Parser::handleListen;
	_directive_handlers["name"]				= &Parser::handleName;
	_directive_handlers["redirect"]			= &Parser::handleRedirect;
	_directive_handlers["root"]				= &Parser::handleRoot;
	_directive_handlers["index"]			= &Parser::handleIndex;
	_directive_handlers["autoindex"]		= &Parser::handleAutoIndex;
	_directive_handlers["methods"]			= &Parser::handleMethods;
	_directive_handlers["max_body_size"]	= &Parser::handleMaxBodySize;
	_directive_handlers["ext"]				= &Parser::handleExt;
	_directive_handlers["script"]			= &Parser::handleScript;
	_directive_handlers["cgi_param"]		= &Parser::handleCgiParam;
	_directive_handlers["error_page"]		= &Parser::handleErrorPage;
}

void Parser::init_parser()
{
	_current_line = 0;
	_current	  = makeToken(NONE, "", _current_line);
	_next		  = makeToken(NONE, "", _current_line);
	init_key_database();
	init_directive_handlers();
}

void Parser::handleDirective(std::vector<t_token> line)
{
	int									 count = 0;
	std::vector<t_token>::iterator		 it	   = line.begin();
	std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);

	if (split == line.end())
		throw Error("Error: invalid config: directive syntax error");
	for (it = line.begin(); it != split; ++it)
		count++;
	if (count != 1) {
		throw Error("Error: invalid config: directive syntax error");
	}
	if (line.begin()->type != KEY)
		throw Error("Error: invalid directive");

	std::map<std::string, Directive_handler>::iterator key =
		_directive_handlers.find(line.front().literal);
	if (key != _directive_handlers.end()) {
		Directive_handler func = key->second;
		(this->*func)(line);
	}
}

void Parser::handleBlockIn(std::vector<t_token> line)
{
	std::vector<t_token>::iterator it = line.begin();
	if (it->literal == "http") {
		_config.setStartLine(_current_line);
		_config.setInBlock(true);
		_current_block = &_config;
	} else if (it->literal == "server") {
		_config.addServer(Server());
		_current_block = &_config.getServers().back();
		_current_block->setParent(&_config);
		_current_block->setStartLine(_current_line);
		_config.setInBlock(true);
	} else if (it->literal == "location") {
		if (_current_block->getBlockType() != SERVER_)
			throw Error("Error: invalid config: location syntax error");
		if (_current_block->isInBlock())
			throw Error("Error: invalid config: location syntax error");
		if (!(it + 1)->literal.empty() && (it + 1)->type != REGEX)
			throw Error("Error: invalid location: missing path");

		Server &temp = getLastServer();
		temp.addLocation(Location());
		_current_block = &temp.getLocations().back();
		_current_block->setParent(&temp);
		_current_block->setStartLine(_current_line);
		Location &temp_loc = *static_cast<Location *>(_current_block);
		temp_loc._path	   = (it + 1)->literal;
		temp.setInBlock(true);
	} else if (it->literal == "cgi") {
		if (_current_block->getBlockType() != LOCATION_)
			throw Error("Error: invalid config: cgi syntax error");
		if (_current_block->isInBlock())
			throw Error("Error: invalid config: cgi syntax error");

		Location &temp = getLastLocation();
		temp._cgi.push_back(CGI());
		_current_block = &temp._cgi.back();
		_current_block->setParent(&temp);
		_current_block->setStartLine(_current_line);
		temp.setInBlock(true);
	}
}

void Parser::handleBlockOut()
{
	if (_current_block->getBlockType() != HTTP_) {
		if (!_current_block->getParent()->isInBlock())
			throw Error("Error: invalid config: block out syntax error");
	}

	if (_current_block->getBlockType() == HTTP_)
		_config.setEndLine(_current_line);
	else if (_current_block->getBlockType() == SERVER_) {
		getLastServer().setEndLine(_current_line);
		_current_block->getParent()->setInBlock(false);
		_current_block = _current_block->getParent();
	} else if (_current_block->getBlockType() == LOCATION_) {
		getLastLocation().setEndLine(_current_line);
		_current_block->getParent()->setInBlock(false);
		_current_block = _current_block->getParent();
	} else if (_current_block->getBlockType() == CGI_) {
		std::string ext	   = getLastCGI()._ext;
		std::string script = getLastCGI()._script;
		if (!ConfigValidator::validateCgiScriptExt(ext, script))
			throw Error("Error: invalid cgi script extension");

		getLastCGI().setEndLine(_current_line);
		_current_block->getParent()->setInBlock(false);
		_current_block = _current_block->getParent();
	}
}

void Parser::tokenise()
{
	ConfigLexer::tokenizeFile(_config_root, _key_database, _tokens, _current_line);
}

void Parser::parse()
{
	_current_line = 0;
	_current_it	  = _tokens.begin();
	while (_current_it != _tokens.end()) {
		_current_line++;
		_current = *_current_it;
		if (_current_it + 1 != _tokens.end())
			_next = *(_current_it + 1);

		unsigned int		 _tmp_current_line = _current.line;
		std::vector<t_token> _line_tokens;
		while (_current_it != _tokens.end() && _current_it->line == _tmp_current_line) {
			_line_tokens.push_back(*_current_it);
			++_current_it;
		}
		_current_line_type = checkLineType(_line_tokens);
		if (_current_line_type == EMPTY)
			continue;
		switch (_current_line_type) {
			case DIRECTIVE:
				handleDirective(_line_tokens);
				break;
			case BLOCK_IN:
				handleBlockIn(_line_tokens);
				break;
			case BLOCK_OUT:
				handleBlockOut();
				break;
			case EMPTY:
				break;
			case ERROR:
				throw Error("Error: parsing: syntax error");
				break;
		}
	}
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

std::string Parser::getConfigRoot() const
{
	return _config_root;
}

unsigned int Parser::getCurrentLine() const
{
	return _current_line;
}

t_token Parser::getCurrentToken() const
{
	return _current;
}

t_token Parser::getNextToken() const
{
	return _next;
}

std::vector<t_token> Parser::getTokens() const
{
	return _tokens;
}

std::vector<t_token>::iterator Parser::getCurrentIt() const
{
	return _current_it;
}

IBlock *Parser::getCurrentBlock() const
{
	return _current_block;
}

HTTP &Parser::getConfig()
{
	return _config;
}

const HTTP &Parser::getConfig() const
{
	return _config;
}

Server &Parser::getLastServer()
{
	return _config.getServers().back();
}

const Server &Parser::getLastServer() const
{
	return _config.getServers().back();
}

Location &Parser::getLastLocation()
{
	return getLastServer().getLocations().back();
}

const Location &Parser::getLastLocation() const
{
	return getLastServer().getLocations().back();
}

CGI &Parser::getLastCGI()
{
	return getLastLocation()._cgi.back();
}

const CGI &Parser::getLastCGI() const
{
	return getLastLocation()._cgi.back();
}

void Parser::setConfigRoot(std::string config_root)
{
	_config_root = config_root;
}

void Parser::setCurrentToken(t_token current)
{
	_current = current;
}

void Parser::setNextToken(t_token next)
{
	_next = next;
}

void Parser::setCurrentIt(std::vector<t_token>::iterator current_it)
{
	_current_it = current_it;
}

void Parser::setCurrentBlock(IBlock *current)
{
	_current_block = current;
}


/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

const char* Parser::Error::what() const throw()
{
	return _msg.c_str();
}

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/

// int main()
// {
//     Parser  newParser("../../resources/webserv.conf");
//
//     try
//     {
//         newParser.init_parser();
//         newParser.tokenise();
//         printTokens(newParser.getTokens());
//         std::cout << std::endl;
//         newParser.parse();
//         newParser.getConfig().printConfig();
//     }
//     catch (std::exception   &e)
//     {
//         std::cerr << e.what() << std::endl;
//     }
// }
