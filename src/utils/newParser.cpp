/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 21:52:43 by margo             #+#    #+#             */
/*   Updated: 2026/01/25 20:32:19 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "newParser.hpp"

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

void    Parser::createKeyDatabase()
{
    std::string line;
    std::fstream    fin;

    fin.open("keywords.txt", std::ios::in);
    while(std::getline(fin, line, '='))
    {
        std::stringstream iss(line);
        std::string key, value;
        
        iss >> key;
        std::getline(fin, value);
        _key_database[key] = value;
    }
}

std::string    Parser::findKeyInDatabase(std::string key, bool value)
{
    std::map<std::string, std::string>::iterator it = _key_database.find(key);

    if (it != _key_database.end())
    {
        if (value)
            return it->second;
        else
            return it->first;
    }
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

std::vector<t_token>::iterator  getTokenFromVector(std::vector<t_token> vec, e_token key)
{
    std::vector<t_token>::iterator it;

    for (it = vec.begin(); it != vec.end(); ++it)
    {
        if (it->type == key)
            return it;
    }
    return vec.end();
}

void    Parser::init_parser()
{
    _current_line = 0;
    _current = makeToken(NONE, "", _current_line);
    _next = makeToken(NONE, "", _current_line);
    _in_block = false;
    createKeyDatabase();
}

// void    Parser::toggle()
// {
//     _current_block->toggle(*this);
// }

void    Parser::handleDirective(std::vector<t_token> line)
{
    Directive new_directive;
    int count = 0;
    std::vector<t_token>::iterator it;
    std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);

    if (split == line.end())
        throw Error("Error: invalid config: syntax error");
    for (it = line.begin(); it != split; ++it)
        count++;
    if (count != 1)
        throw Error("Error: invalid config: syntax error");
    --it;

    new_directive._key = it->literal;
    new_directive._line = it->line;
    for (it = split + 1; it != line.end(); ++it)
        _parameter.push_back(it->literal);
}

// void    Parser::handleDirective(std::vector<t_token> line)
// {
//     std::vector<t_token>::iterator it;
//     std::vector<t_token>::iterator split = getTokenFromVector(line, EQUAL);
//     int count = 0;

//     if (split == line.end())
//         throw Error("Error: handling directive during parsing");
    
//     for (it = line.begin(); it != split; ++it)
//         count++;
//     if (count != 1)
//         throw Error("Error: handling directive: too many key words on line");
    
//     --it;
//     if (it->literal == "listen")
//     {
//         int port;
//         std::stringstream iss((split + 1)->literal);
//         iss >> port;
//         if (port < 1 || port > 65636)
//             throw Error("Error: invalid config: port");
//         _config.http.server.ipv4_listen.sin_port = htons(port);
//         ++split;
//         if (split->type != SEMICOLON)
//             throw Error("Error: invalid config: syntax error");
//     }
//     else if (it->literal == "name")
//     {
//         std::string hostname = (split + 1)->literal;
//         _config.http.server.server_name = hostname;
//         ++split;
//         if (split->type != SEMICOLON)
//             throw Error("Error: invalid config: syntax error");
//     }
//     else if (it->literal == "root")
//     {
//         std::string root_path = (split + 1)->literal;
//         _config.http.server.location.path = root_path;
//         ++split;
//         if (split->type != SEMICOLON)
//             throw Error("Error: invalid config: syntax error");
//     }
//     else if (it->literal == "index")
//     {
//         it = split + 1;
//         while (it->type != SEMICOLON)
//         {
//             std::string index = it->literal;
//             _config.http.server.location.config.index.push_back(index);
//             it++;
//         }
//     }
//     else if (it->literal == "autoindex")
//     {
//         bool    autoidx;
//         if ((it + 1)->literal == "true")
//             autoidx = true;
//         else
//             autoidx = false;
//         _config.http.server.location.config.autoindex = autoidx;
//     }
//     else if (it->literal == "methods")
//     {
        
//     }
// }

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
        Server  new_server;
        new_server.setParent(&_config);
        new_server.setStartLine(_current_line);
        _current_block = &new_server;
        _config.addServer(new_server);
        _config.setInBlock(true);
        _in_block = true;
    }
    else if (it->literal == "location")
    {
        if (_current_block->getBlockType() != SERVER_)
            throw Error("Error: invalid config: syntax error");
        if (_current_block->isInBlock())
            throw Error("Error: invalid config: syntax error");
        
        Location    new_location;
        new_location.setParent(_current_block);
        new_location.setStartLine(_current_line);
        _current_block = &new_location;
        getLastServer().addLocation(new_location);
        getLastServer().setInBlock(true);
    }
    else if (it->literal == "cgi")
    {
        if (_current_block->getBlockType() != LOCATION_)
            throw Error("Error: invalid config: syntax error");
        if (_current_block->isInBlock())
            throw Error("Error: invalid config: syntax error");

        CGI new_cgi;
        new_cgi.setParent(_current_block);
        new_cgi.setStartLine(_current_line);
        _current_block = &new_cgi;
        getLastLocation()._cgi.push_back(new_cgi);
        getLastLocation().setInBlock(true);
    }
}

void    Parser::handleBlockOut(std::vector<t_token> line)
{
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
            if (findKeyInDatabase(word, false) != "")
				_tokens.push_back(makeToken(KEY, word, _current_line));
			else if (word == "{")
				_tokens.push_back(makeToken(BLOCK_START, word, _current_line));
			else if (word == "}")
				_tokens.push_back(makeToken(BLOCK_END, word, _current_line));
			else if (word == "=")
                _tokens.push_back(makeToken(EQUAL, word, _current_line));
            else if (word.find('/') != std::string::npos)
				_tokens.push_back(makeToken(REGEX, word, _current_line));
			else if (word[0] == '"' && word[word.length() - 1] == '"')
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
    for (_current_it = _tokens.begin(); _current_it != _tokens.end(); ++_current_it)
    {
        _current = *_current_it;
        if (_current_it + 1 != _tokens.end())
            _next = *(_current_it + 1);

        unsigned int _tmp_current_line = _current.line;
        std::vector<t_token> _line_tokens;
        std::vector<t_token>::iterator _tmp_it;
        for (_tmp_it = _current_it; _tmp_it->line != _tmp_current_line; ++_tmp_it)
            _line_tokens.push_back(*_tmp_it);
        _current_line_type = checkLineType(_line_tokens);
        switch (_current_line_type)
        {
            case DIRECTIVE:
                break ;
            case BLOCK_IN:
                handleBlockIn(_line_tokens);
                break ;
            case BLOCK_OUT:
                break ;
            case EMPTY:
                break ;
            case ERROR:
                break ;
        }
    }
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

int main()
{
    Parser  newParser("../../resources/webserv.conf");

    newParser.init_parser();
    newParser.tokenise();
    printTokens(newParser.getTokens());
}