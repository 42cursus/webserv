/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newState.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/25 15:43:47 by margo             #+#    #+#             */
/*   Updated: 2026/01/25 20:14:47 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "newState.hpp"

Directive& Directive::operator=(const Directive& copy)
{
    if (this != &copy)
    {
        _key = copy._key;
        _parameter = copy._parameter;
        //_equal = copy._equal;
        _line = copy._line;
    }
    return *this;
}

IBlock::IBlock(e_block_type type): _in_block(false), _type(type), _line_start(0), _line_end(0) {}

IBlock::IBlock(const IBlock& copy)
{
    _in_block = copy._in_block;
    _type = copy._type;
    _line_start = copy._line_start;
    _line_end = copy._line_end;
    _parent_block = copy._parent_block;
    for (size_t i = 0; i < copy._directives.size(); i++)
        _directives[i] = copy._directives[i];
}

IBlock& IBlock::operator=(const IBlock& copy)
{
    if (this != &copy)
    {
        _in_block = copy._in_block;
        _type = copy._type;
        _line_start = copy._line_start;
        _line_end = copy._line_end;
        _parent_block = copy._parent_block;
        for (size_t i = 0; i < copy._directives.size(); i++)
            _directives[i] = copy._directives[i];
    }
    return *this;
}

bool    IBlock::operator==(IBlock& oth)
{
    return (_in_block == oth._in_block && _line_start == oth._line_start && _line_end == oth._line_end && _type == oth._type && _parent_block == oth._parent_block && _directives == oth._directives);
}

IBlock::~IBlock() { delete _parent_block; }

bool    IBlock::isInBlock() const { return _in_block; }

unsigned int    IBlock::getStartLine() const { return _line_start; }

unsigned int IBlock::getEndLine() const { return _line_end; }

e_block_type IBlock::getBlockType() const { return _type; }

IBlock* IBlock::getParent() const { return _parent_block; }

std::vector<Directive> IBlock::getDirectives() const { return _directives; }

void    IBlock::setInBlock(bool in_block) { _in_block = in_block; }

void    IBlock::setStartLine(unsigned int line_start) { _line_start = line_start; };

void    IBlock::setEndLine(unsigned int line_end) { _line_end = line_end; }

void    IBlock::setParent(IBlock* parent) { _parent_block = parent; }

void    IBlock::addDirective(Directive new_directive) { _directives.push_back(new_directive); }

CGI::CGI(): IBlock(CGI_) {}

CGI& CGI::operator=(const CGI& copy)
{
    if (this != &copy)
    {
        IBlock::operator=(copy);
        _ext = copy._ext;
        _script = copy._script;
    }
    return *this;
}

Location::Location(): IBlock(LOCATION_) {}

Location&   Location::operator=(const Location& copy)
{
    if (this != &copy)
    {
        IBlock::operator=(copy);
        _root = copy._root;
        _path = copy._path;
        for (size_t i = 0; i < copy._index.size(); i++)
            _index[i] = copy._index[i];
        for (size_t i = 0; i < copy._methods.size(); i++)
            _methods[i] = copy._methods[i];
        _max_body_size = copy._max_body_size;
        _autoindex = copy._autoindex;
        for (size_t i = 0; i < copy._cgi.size(); i++)
            _cgi[i] = copy._cgi[i];
    }
    return *this;
}

Server::Server(): IBlock(SERVER_), _port(-1) {}

Server::Server(const Server& copy): IBlock(copy)
{
    _port = copy._port;
    _hostname = copy._hostname;
    for (size_t i = 0; i < copy._locations.size(); i++)
        _locations[i] = copy._locations[i];
    _error_pages = std::map<std::string, std::string>(copy._error_pages);
}

Server& Server::operator=(const Server& copy)
{
    if (this != &copy)
    {
        IBlock::operator=(copy);
        _port = copy._port;
        _hostname = copy._hostname;
        for (size_t i = 0; i < copy._locations.size(); i++)
            _locations[i] = copy._locations[i];
        _error_pages = std::map<std::string, std::string>(copy._error_pages);
    }
    return *this;
}

Server::~Server() {}

unsigned int Server::getPort() const { return _port; }

std::string Server::getHost() const { return _hostname; }

std::vector<Location> Server::getLocations() const { return _locations; }

std::map<std::string, std::string>  Server::getErrorPages() const { return _error_pages; }

void    Server::setPort(unsigned int port) { _port = port; }

void    Server::setHost(std::string hostname) { _hostname = hostname; }

void    Server::addLocation(Location& new_location) { _locations.push_back(new_location); }

void    Server::addErrorPage(std::string code, std::string html) { _error_pages[code] = html; }

HTTP::HTTP(): IBlock(HTTP_) {}

HTTP::HTTP(const HTTP& copy): IBlock(copy)
{
    _workers = copy._workers;
    _log_format = copy._log_format;
    for (size_t i = 0; i < copy._servers.size(); i++)
        _servers[i] = copy._servers[i];
}

HTTP& HTTP::operator=(const HTTP& copy)
{
    if (this != &copy)
    {
        IBlock::operator=(copy);
        _workers = copy._workers;
        _log_format = copy._log_format;
        for (size_t i = 0; i < copy._servers.size(); i++)
            _servers[i] = copy._servers[i];
    }
    return *this;
}

HTTP::~HTTP() {};

std::string HTTP::getWorkers() const { return _workers; }

std::string HTTP::getLogFormat() const { return _log_format; }

std::vector<Server> HTTP::getServers() const { return _servers; }

void    HTTP::setWorkers(std::string workers) { _workers = workers; }

void    HTTP::setLogFormat(std::string log_format) { _log_format = log_format; }

void    HTTP::addServer(Server new_server) { _servers.push_back(new_server); }
