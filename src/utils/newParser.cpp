/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 21:52:43 by margo             #+#    #+#             */
/*   Updated: 2026/01/26 11:13:47 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "newParser.hpp"
#include "newState.hpp"

std::string	Parser::readQuotedString(std::string word)
{
	if (word.length() >= 2 && word[0] == '"' && word[word.size() - 1] == '"')
		return word.substr(1, word.length() - 2);

	return word;
}

bool    t_token::operator==(const t_token& other) const
{
    return (type == other.type && literal == other.literal && line == other.line);
}

t_token Parser::makeToken(e_token key, std::string word, int linecount)
{
    t_token token;

    token.type = key;
    token.literal = word;
    token.line = linecount;

    return token;
}

std::string    Parser::findKeyInDatabase(std::string key)
{
    std::vector<std::string>::iterator it = std::find(_key_database.begin(), _key_database.end(), key);

    if (it != _key_database.end())
        return *it;
    return "";
}

e_line_type Parser::checkLineType(std::vector<t_token> line)
{
    if (line.empty())
        return EMPTY;
    else if (line.rbegin()->type == BLOCK_START)
        return BLOCK_IN;
    else if (line.rbegin()->type == BLOCK_END)
        return BLOCK_OUT;
    else if (line.rbegin()->type == SEMICOLON)
        return DIRECTIVE;
    else
        return ERROR;
}

std::vector<t_token>::iterator  Parser::getTokenFromVector(std::vector<t_token> vec, e_token key)
{
    std::vector<t_token>::iterator it;

    for (it = vec.begin(); it != vec.end(); ++it)
    {
        if (it->type == key)
            return it;
    }
    return vec.end();
}

void    Parser::handleWorkers(const std::vector<t_token> line)
{
    //if (line.size() < 4 || _current_block->getBlockType() != HTTP_)
    //    throw Error("Error: invalid directive: worker_processes");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    _config.setWorkers((++split)->literal);
}

void    Parser::handleLogFormat(const std::vector<t_token> line)
{
    // if (line.size() < 4 || _current_block->getBlockType() != HTTP_)
    //     throw Error("Error: invalid directive: log_format");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    _config.setLogFormat((++split)->literal);
}

void    Parser::handleListen(const std::vector<t_token> line)
{
    // if (line.size() < 4 || _current_block->getBlockType() != SERVER_)
    //     throw Error("Error: invalid directive: listen");
    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    std::stringstream iss((++split)->literal);
    unsigned int port = 0;
    
    iss >> port;
    if (port < 1 || port > 65636)
        throw Error("Error: invalid config: port");
    getLastServer().setPort(port);
}

void    Parser::handleName(const std::vector<t_token> line)
{
    // if (line.size() < 4 || _current_block->getBlockType() != SERVER_)
    //     throw Error("Error: invalid directive: hostname");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    getLastServer().setHost((++split)->literal);
}

void    Parser::handleRoot(const std::vector<t_token> line)
{
    // if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
    //     throw Error("Error: invalid directive: root");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    if ((++split)->type != REGEX)
        throw Error("Error: syntax error: root");
    getLastLocation()._root = split->literal;
}

void    Parser::handleIndex(const std::vector<t_token> line)
{
    // if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
    //     throw Error("Error: invalid directive: index");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    for (++split; split->type != SEMICOLON; ++split)
        getLastLocation()._index.push_back(split->literal); 
}

void    Parser::handleAutoIndex(const std::vector<t_token> line)
{
    // if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
    //     throw Error("Error: invalid directive: autoindex");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    getLastLocation()._autoindex = ((++split)->literal == "true" ? true : false);
}

void    Parser::handleMethods(const std::vector<t_token> line)
{
    // if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
    //     throw Error("Error: invalid directive: methods");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    for (++split; split->type != SEMICOLON; ++split)
        getLastLocation()._methods.push_back(split->literal);
}

void    Parser::handleMaxBodySize(const std::vector<t_token> line)
{
    // if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
    //     throw Error("Error: invalid directive: max_body_size");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    getLastLocation()._max_body_size = atoll((++split)->literal.c_str());
}

void    Parser::handleExt(const std::vector<t_token> line)
{
    // if (line.size() < 4 || _current_block->getBlockType() != CGI_)
    //     throw Error("Error: invalid directive: ext");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    getLastCGI()._ext = (++split)->literal;
}

void    Parser::handleScript(const std::vector<t_token> line) // check if ext on script is the same as _ext of cgi
{
    // if (line.size() < 4 || _current_block->getBlockType() != CGI_)
    //     throw Error("Error: invalid directive: script");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    getLastCGI()._script = (++split)->literal;
}

void    Parser::handleErrorPage(const std::vector<t_token> line) // check if last token before semicolon is a .html file
{
    // if (line.size() < 5 || _current_block->getBlockType() != SERVER_)
    //     throw Error("Error: invalid directive: error_page");

    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
    std::vector<t_token>::iterator html = getTokenFromVector(line,  SEMICOLON);
    --html;
    for (++split; split != html; ++split)
        getLastServer().addErrorPage(split->literal, html->literal);
}

void    Parser::init_key_database()
{
    std::string line;
    std::fstream    fin;

    fin.open("keywords.txt", std::ios::in);
    while(std::getline(fin, line))
    {
        std::stringstream iss(line);
        std::string key;
        
        iss >> key;
        _key_database.push_back(key);
    }
    fin.close();
}

void    Parser::init_directive_handlers()
{
    _directive_handlers["worker_processes"] = &Parser::handleWorkers;
    _directive_handlers["log_format"] = &Parser::handleLogFormat;
    _directive_handlers["listen"] = &Parser::handleListen;
    _directive_handlers["name"] = &Parser::handleName;
    _directive_handlers["root"] = &Parser::handleRoot;
    _directive_handlers["index"] = &Parser::handleIndex;
    _directive_handlers["autoindex"] = &Parser::handleAutoIndex;
    _directive_handlers["methods"] = &Parser::handleMethods;
    _directive_handlers["max_body_size"] = &Parser::handleMaxBodySize;
    _directive_handlers["ext"] = &Parser::handleExt;
    _directive_handlers["script"] = &Parser::handleScript;
    _directive_handlers["error_page"] = &Parser::handleErrorPage;
}

void    Parser::init_parser()
{
    _current_line = 0;
    _current = makeToken(NONE, "", _current_line);
    _next = makeToken(NONE, "", _current_line);
    _in_block = false;
    init_key_database();
    init_directive_handlers();
}

void    Parser::handleDirective(std::vector<t_token> line)
{
    //int count = 0;
    std::cout << "HELLO" << std::endl;
    std::vector<t_token>::iterator it = line.begin();
    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);

    if (split == line.end())
        throw Error("Error: invalid config: syntax error");
    // for (it = line.begin(); it != split; ++it)
    //     count++;
    // if (count != 1)
    //     throw Error("Error: invalid config: syntax error");
    if (it->type != KEY)
        throw Error("Error: invalid directive");

    std::map<std::string, Directive_handler>::iterator key = _directive_handlers.find(line.front().literal);
    if (key != _directive_handlers.end())
    {
        Directive_handler func = key->second;
        (this->*func)(line);
    }        
}

void    Parser::handleBlockIn(std::vector<t_token> line)
{
    std::vector<t_token>::iterator  it = line.begin();
    if (it->literal == "http")
    {
        _config.setStartLine(_current_line);
        _config.setInBlock(true);
    }
    else if (it->literal == "server")
    {
        _config.addServer(Server());
        _current_block = &_config.getServers().back();
        _current_block->setParent(&_config);
        _current_block->setStartLine(_current_line);
        _config.setInBlock(true);
        _in_block = true;
    }
    else if (it->literal == "location")
    {
        if (_current_block->getBlockType() != SERVER_)
            throw Error("Error: invalid config: syntax error");
        if (_current_block->isInBlock())
            throw Error("Error: invalid config: syntax error");
        
        Server& temp = getLastServer();
        temp.addLocation(Location());
        _current_block = &temp.getLocations().back();
        _current_block->setParent(&temp);
        _current_block->setStartLine(_current_line);
        Location& temp_loc = *static_cast<Location*>(_current_block);
        temp_loc._path = (++it)->literal;
        temp.setInBlock(true);
    }
    else if (it->literal == "cgi")
    {
        if (_current_block->getBlockType() != LOCATION_)
            throw Error("Error: invalid config: syntax error");
        if (_current_block->isInBlock())
            throw Error("Error: invalid config: syntax error");

        Location&   temp = getLastLocation();
        temp._cgi.push_back(CGI());
        _current_block = &temp._cgi.back();
        _current_block->setParent(&temp);
        _current_block->setStartLine(_current_line);
        temp.setInBlock(true);
    }
}

void    Parser::handleBlockOut(std::vector<t_token> line)
{
    (void)line;
    if (!_current_block->isInBlock())
        throw Error("Error: invalid config: syntax error");

    if (_current_block->getBlockType() == HTTP_)
    {
        _config.setEndLine(_current_line);
    }
    else if (_current_block->getBlockType() == SERVER_)
    {
        getLastServer().setEndLine(_current_line);
        _current_block->getParent()->setInBlock(false);
    }
    else if (_current_block->getBlockType() == LOCATION_)
    {
        getLastLocation().setEndLine(_current_line);
        _current_block->getParent()->setInBlock(false);
    }
    else if (_current_block->getBlockType() == CGI_)
    {
        getLastCGI().setEndLine(_current_line);
        _current_block->getParent()->setInBlock(false);
    }
}


void    Parser::tokenise()
{
    std::string line;
    std::ifstream file(_config_root.c_str());
    
    try {
        file.is_open();
    }
    catch (std::exception&  e) {
        std::cerr << e.what() << std::endl;
    }
    
    while (std::getline(file, line))
    {
        std::istringstream  iss(line);
        std::string word;

        _current_line++;
        if (line.empty())
            continue ;
        
        while (iss >> word)
        {
            if (word[0] == '#')
            {
                _tokens.push_back(makeToken(COMMENT, word, _current_line));
                break ;
            }
            if (findKeyInDatabase(word) != "")
				_tokens.push_back(makeToken(KEY, word, _current_line));
			else if (word == "{")
				_tokens.push_back(makeToken(BLOCK_START, word, _current_line));
			else if (word == "}")
				_tokens.push_back(makeToken(BLOCK_END, word, _current_line));
			else if (word == "=")
                _tokens.push_back(makeToken(EQUAL, word, _current_line));
            else if (word.find('/') != std::string::npos)
				_tokens.push_back(makeToken(REGEX, word, _current_line));
			else if (word[0] == '"' && word[word.length() - 1] == '"') // need a better way to handle quoted strings for log_format
				_tokens.push_back(makeToken(QUOTES, readQuotedString(word), _current_line));
			else if (word[0] == '$')
				_tokens.push_back(makeToken(VAR, word, _current_line)); // to do: function extracting/expanding variable ???
			else if (word == ";" || word[word.length() - 1] == ';')
			{
				if (word != ";")
					_tokens.push_back(makeToken(KEY, word.substr(0, word.length() - 1), _current_line));
				_tokens.push_back(makeToken(SEMICOLON, ";", _current_line));
			}
			else if (word[word.length() - 1] == '\n' || word == "\n")
			{
				if (word != "\n")
					_tokens.push_back(makeToken(KEY, word.substr(0, word.length() - 1), _current_line));
				_tokens.push_back(makeToken(EOL, "\n", _current_line));
			}
			else
				_tokens.push_back(makeToken(ILLEGAL, word, _current_line));
        }
    }
    file.close();
}

void    Parser::parse()
{
    setCurrentBlock(new Server());
    _current_it = _tokens.begin();
    while (_current_it != _tokens.end())
    {
        _current = *_current_it;
        if (_current_it + 1 != _tokens.end())
            _next = *(_current_it + 1);

        unsigned int _tmp_current_line = _current.line;
        std::cout << "line " << _current.line << " " << _tmp_current_line << std::endl;
        std::vector<t_token> _line_tokens;
        while (_current_it != _tokens.end() && _current_it->line == _tmp_current_line)
        {
            _line_tokens.push_back(*_current_it);
            ++_current_it;
        }
        _current_line_type = checkLineType(_line_tokens);
        if (_current_line_type == EMPTY)
            continue ;
        std::cout << "current token " << _current_it->literal << std::endl;
        switch (_current_line_type)
        {
            case DIRECTIVE:
                handleDirective(_line_tokens);
                break ;
            case BLOCK_IN:
                handleBlockIn(_line_tokens);
                break ;
            case BLOCK_OUT:
                handleBlockOut(_line_tokens);
                break ;
            case EMPTY:
                break ;
            case ERROR:
                throw Error("Error: parsing: syntax error");
                break ;
        }
    }
    std::cout << "exit loop" << std::endl;
}

void	printTokens(std::vector<t_token> tokens)
{
	for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
	{
		if (it->type == EOL)
			std::cout << "EOL " << it->literal << " on line " << it->line << std::endl;
		else if (it->type == EQUAL)
            std::cout << "EQUAL " << it->literal << " on line " << it->line << std::endl;
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

void    HTTP::printConfig()
{
    std::cout << "HTTP server config:" << std::endl
              << "worker_processes = " << _workers << std::endl
              << "log_format = " << _log_format << std::endl;
    for (size_t i = 0; i < _servers.size(); i++)
    {
        std::cout << "Server " << i + 1 << " config:" << std::endl
                  << "port = " << _servers[i].getPort() << std::endl
                  << "hostname = " << _servers[i].getHost() << std::endl
                  << "error_pages: " << std::endl;
        std::map<std::string, std::string>::iterator it;
        for (it = _servers[i].getErrorPages().begin(); it != _servers[i].getErrorPages().end(); ++it)
            std::cout << "page code " << it->first << " uses html script " << it->second << std::endl;
        for (size_t j = 0; j < _servers[i].getLocations().size(); j++)
        {
            Location current = _servers[i].getLocations()[j];
            std::cout << "Location " << j + 1 << " config:" << std::endl
                      << "path = " << current._path << std::endl
                      << "root = " << current._root << std::endl
                      << "methods = ";
            for (size_t k = 0; k < current._index.size(); k++)
                std::cout << current._methods[k] << " ";
            std::cout << std::endl << "index = ";
            for (size_t k = 0; k < current._index.size(); k++)
                std::cout << current._index[k] << " ";
            std::cout << std::endl << "max_body_size = " << current._max_body_size << std::endl
                      << "autoindex = " << (current._autoindex ? "true" : "false") << std::endl;
            for (size_t l = 0; l < current._cgi.size(); l++)
            {
                std::cout << "CGI " << l + 1 << " config:" << std::endl
                          << "ext = " << current._cgi[l]._ext << std::endl
                          << "script = " << current._cgi[l]._script << std::endl; 
            }
        }
    }
}

int main()
{
    Parser  newParser("../../resources/webserv.conf");

    newParser.init_parser();
    newParser.tokenise();
    printTokens(newParser.getTokens());
    std::cout << std::endl;
    newParser.parse();
    newParser.getConfig().printConfig();
}