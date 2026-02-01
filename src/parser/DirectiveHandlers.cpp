/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectiveHandlers.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 17:16:32 by margo             #+#    #+#             */
/*   Updated: 2026/02/01 17:53:42 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "State.hpp"

void    Parser::handleWorkers(const std::vector<t_token> line)
{
    if (line.size() < 4)
        throw Error("Error: invalid directive: worker_processes");
    if (_current_block->getBlockType() != HTTP_)
        throw Error("Error: directive worker_processes not in HTTP block");
    
    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    _config.setWorkers((++split)->literal);
}

void    Parser::handleLogFormat(const std::vector<t_token> line)
{
    if (line.size() < 4 || _current_block->getBlockType() != HTTP_)
        throw Error("Error: invalid directive: log_format");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    _config.setLogFormat((++split)->literal);
}

void    Parser::handleListen(const std::vector<t_token> line)
{
    if (line.size() < 4 || _current_block->getBlockType() != SERVER_)
        throw Error("Error: invalid directive: listen");
    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    std::stringstream iss((++split)->literal);
    unsigned int port = 0;
    
    iss >> port;
    if (port < 1 || port > 65636)
        throw Error("Error: invalid config: port");
    getLastServer().setPort(port);
}

void    Parser::handleName(const std::vector<t_token> line)
{
    if (line.size() < 4 || _current_block->getBlockType() != SERVER_)
        throw Error("Error: invalid directive: hostname");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    getLastServer().setHost((++split)->literal);
}

void    Parser::handleRedirect(const std::vector<t_token> line)
{
    if (line.size() < 5 || _current_block->getBlockType() != SERVER_)
        throw Error("Error: invalid directive: redirect");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    
    if ((++split)->type != REGEX || (++split)->type != REGEX)
        throw Error("Error: invalid directive: redirect: invalid path/redirect");
    if (!validateErrorCode((++split)->literal))
        throw Error("Error: invalid directive: redirect: invalid page code");
    if (split->literal[0] != '3')
        throw Error("Error: invalid directive: redirect: invalid page code");

    split = getTokenFromVector(line, EQUAL);
    std::string path = (++split)->literal;
    std::string redirect = (++split)->literal;
    StatusCode  code = static_cast<StatusCode>(atoi((++split)->literal.c_str()));
    getLastServer().addRedirect(path, redirect, code);
}

void    Parser::handleRoot(const std::vector<t_token> line)
{
    if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
        throw Error("Error: invalid directive: root");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    
    if ((++split)->type != REGEX)
        throw Error("Error: syntax error: root");
    if (!validateLocationRedirect(getLastLocation()._path, split->literal))
        throw Error("Error: syntax error: root doesn't match path");
    
    getLastLocation()._root = split->literal;
}

void    Parser::handleIndex(const std::vector<t_token> line)
{
    if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
        throw Error("Error: invalid directive: index");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    for (++split; split->type != SEMICOLON; ++split)
        getLastLocation()._index.push_back(split->literal); 
}

void    Parser::handleAutoIndex(const std::vector<t_token> line)
{
    if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
        throw Error("Error: invalid directive: autoindex");        
        
    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    
    if (!validateBool((++split)->literal))
        throw Error("Error: invalid boolean: autoindex");
    
    getLastLocation()._autoindex = (split->literal == "true" ? true : false);
}

void    Parser::handleMethods(const std::vector<t_token> line)
{
    if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
        throw Error("Error: invalid directive: methods");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    for (++split; split->type != SEMICOLON; ++split)
    {
        if (!validateMethod(split->literal))
            throw Error("Error: invalid method");
        
        getLastLocation()._methods.push_back(split->literal);
    }
}

void    Parser::handleMaxBodySize(const std::vector<t_token> line)
{
    if (line.size() < 4 || _current_block->getBlockType() != LOCATION_)
        throw Error("Error: invalid directive: max_body_size");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    getLastLocation()._max_body_size = atoll((++split)->literal.c_str());
}

void    Parser::handleExt(const std::vector<t_token> line)
{
    if (line.size() < 4 || _current_block->getBlockType() != CGI_)
        throw Error("Error: invalid directive: ext");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    getLastCGI()._ext = (++split)->literal;
}

void    Parser::handleScript(const std::vector<t_token> line) // check if ext on script is the same as _ext of cgi
{
    if (line.size() < 4 || _current_block->getBlockType() != CGI_)
        throw Error("Error: invalid directive: script");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    getLastCGI()._script = (++split)->literal;
}

void    Parser::handleCgiParam(const std::vector<t_token> line)
{
    if (line.size() < 5 || _current_block->getBlockType() != CGI_)
        throw Error("Error: invalid directive: cgi_param");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    std::string key = (++split)->literal;
    std::string value = (++split)->literal;
    
    if (!validateCgiParam(value))
        throw Error("Error: invalid cgi_param value");
    
    getLastCGI()._cgi_param[key] = value;
}

void    Parser::handleErrorPage(const std::vector<t_token> line) // check if last token before semicolon is a .html file
{
    if (line.size() < 5 || _current_block->getBlockType() != SERVER_)
        throw Error("Error: invalid directive: error_page");

    std::vector<t_token>::const_iterator split = getTokenFromVector(line, EQUAL);
    std::vector<t_token>::const_iterator html = getTokenFromVector(line,  SEMICOLON);
    --html;
    
    if (!validateErrorPage(html->literal))
        throw Error("Error: invalid error page html file");
    
    for (++split; split != html; ++split)
    {
        if (!validateErrorCode(split->literal))
            throw Error("Error: invalid error page code");
        getLastServer().addErrorPage(split->literal, html->literal);
    }
}