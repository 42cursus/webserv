/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ParserUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:22:22 by margo             #+#    #+#             */
/*   Updated: 2026/02/01 17:54:47 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "State.hpp"

void Parser::readLogFormatString(std::ifstream& file, std::istringstream& iss, std::string& word)
{
    _tokens.push_back(makeToken(KEY, word, _current_line));
    iss >> word;
    _tokens.push_back(makeToken(EQUAL, word, _current_line));
    std::string token_literal;
    std::string line;
    
    while (iss >> word)
    {
        token_literal += word;
        token_literal += " ";
    }
    while (std::getline(file, line))
    {
        if (line[line.length() - 1] == ';' || line == ";")
        {
            line = line.substr(0, line.length() - 1);
            token_literal += line;
            break;
        }
        token_literal += line;
    }
    _tokens.push_back(makeToken(KEY, token_literal, _current_line));
    _tokens.push_back(makeToken(SEMICOLON, ";", _current_line));
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

std::vector<t_token>::const_iterator  Parser::getTokenFromVector(const std::vector<t_token>& vec, e_token key)
{
    std::vector<t_token>::const_iterator it;

    for (it = vec.begin(); it != vec.end(); ++it)
    {
        if (it->type == key)
            return it;
    }
    return vec.end();
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
        std::map<std::string, std::string>::const_iterator it;
        for (it = _servers[i].getErrorPages().begin(); it != _servers[i].getErrorPages().end(); ++it)
            std::cout << "page code " << it->first << " uses html script " << it->second << std::endl;
        std::vector<Redirect>::const_iterator it_redirect;
        for (it_redirect = _servers[i].getRedirects().begin(); it_redirect != _servers[i].getRedirects().end(); ++it_redirect)
            std::cout << "redirect: " << *it_redirect << std::endl;
        for (size_t j = 0; j < _servers[i].getLocations().size(); j++)
        {
            Location current = _servers[i].getLocations()[j];
            std::cout << "Location " << j + 1 << " config:" << std::endl
                      << "path = " << current._path << std::endl
                      << "root = " << current._root << std::endl
                      << "methods = ";
            for (size_t k = 0; k < current._methods.size(); k++)
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
                          << "script = " << current._cgi[l]._script << std::endl
                          << "cgi_params:" << std::endl;
                std::map<std::string, std::string>::const_iterator it;
                for (it = current._cgi[l]._cgi_param.begin(); it != current._cgi[l]._cgi_param.end(); ++it)
                    std::cout << "key = " << it->first << " value = " << it->second << std::endl;
            }
        }
    }
}

void	printTokens(std::vector<t_token> tokens)
{
	for (std::vector<t_token>::iterator it = tokens.begin(); it != tokens.end(); ++it)
	{
		if (it->type == EQUAL)
            std::cout << "EQUAL " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == KEY)
			std::cout << "KEYWORD " << it->literal << " on line " << it->line << std::endl;
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
		else if (it->type == ILLEGAL)
			std::cout << "ILLEGAL " << it->literal << " on line " << it->line << std::endl;
		else
			std::cout << "UNKNOWN " << it->literal << " on line " << it->line << std::endl;
	}
}
