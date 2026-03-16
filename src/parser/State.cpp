/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   State.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/25 15:43:47 by margo             #+#    #+#             */
/*   Updated: 2026/02/01 17:53:15 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "ConfigCompiler.hpp"
#include "Location.hpp"
#include "State.hpp"
#include "Prefix_suffix.hpp"
#include <vector>

IBlock::IBlock(e_block_type type): _in_block(false), _type(type), _line_start(0), _line_end(0) {}

IBlock::IBlock(const IBlock& copy)
{
    _in_block = copy._in_block;
    _type = copy._type;
    _line_start = copy._line_start;
    _line_end = copy._line_end;
    _parent_block = copy._parent_block;
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
    }
    return *this;
}

bool    IBlock::operator==(IBlock& oth)
{
    return (_in_block == oth._in_block && _line_start == oth._line_start && _line_end == oth._line_end && _type == oth._type && _parent_block == oth._parent_block);
}

IBlock::~IBlock() {}

bool    IBlock::isInBlock() const { return _in_block; }

unsigned int    IBlock::getStartLine() const { return _line_start; }

unsigned int IBlock::getEndLine() const { return _line_end; }

e_block_type IBlock::getBlockType() const { return _type; }

IBlock* IBlock::getParent() const { return _parent_block; }

void    IBlock::setInBlock(bool in_block) { _in_block = in_block; }

void    IBlock::setStartLine(unsigned int line_start) { _line_start = line_start; };

void    IBlock::setEndLine(unsigned int line_end) { _line_end = line_end; }

void    IBlock::setParent(IBlock* parent) { _parent_block = parent; }

IBlock::IBlock() : _in_block(false) {
}

Redirect&   Redirect::operator=(const Redirect& copy)
{
    if (this != &copy)
    {
        _path = copy._path;
        _redirect = copy._redirect;
        _code = copy._code;
    }
    return *this;
}

std::ostream &operator<<(std::ostream &os, const Redirect &redirect)
{
    os << "_path: " << redirect._path << "_redirect: " << redirect._redirect << "_code: " << redirect._code;
    return os;
}

CGI::CGI(): IBlock(CGI_)
{
	setInBlock(false);
}

CGI& CGI::operator=(const CGI& copy)
{
    if (this != &copy)
    {
        IBlock::operator=(copy);
        _ext = copy._ext;
        _script = copy._script;
        _cgi_param = std::map<std::string, std::string>(copy._cgi_param);
    }
    return *this;
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& m)
{
    os << "{";

    typename std::map<K, V>::const_iterator it = m.begin();
    while (it != m.end()) {
        os << it->first << ": " << it->second;

        typename std::map<K, V>::const_iterator next = it;
        ++next;
        if (next != m.end()) {
            os << ", ";
        }

        it = next;
    }

    os << "}";
    return os;
}

std::ostream &operator<<(std::ostream &os, const CGI &cgi) {
    os << " _ext: " << cgi._ext << " _script: " << cgi._script << " _cgi_param: " << cgi._cgi_param;
    return os;
}

Server::Server(): IBlock(SERVER_), _port(-1)
{
	setInBlock(false);
}

Server::Server(const Server& copy): IBlock(copy)
{
    _port = copy._port;
    _hostname = copy._hostname;
    for (size_t i = 0; i < copy._locations.size(); i++)
        _locations.push_back(copy._locations[i]);
    for (size_t i = 0; i < copy._redirects.size(); i++)
        _redirects.push_back(copy._redirects[i]);
    _error_pages = std::map<std::string, std::string>(copy._error_pages);
}

Server& Server::operator=(const Server& copy)
{
    if (this != &copy)
    {
        IBlock::operator=(copy);
        _port = copy._port;
        _hostname = copy._hostname;
        _locations.clear();
        for (size_t i = 0; i < copy._locations.size(); i++)
            _locations.push_back(copy._locations[i]);
        _redirects.clear();
        for (size_t i = 0; i < copy._redirects.size(); i++)
            _redirects.push_back(copy._redirects[i]);
        _error_pages = std::map<std::string, std::string>(copy._error_pages);
    }
    return *this;
}

Server::~Server() {}

unsigned int Server::getPort() const { return _port; }

std::string Server::getHost() const { return _hostname; }

std::vector<Location>& Server::getLocations() { return _locations; }

const std::vector<Location>& Server::getLocations() const { return _locations; }

std::vector<Redirect>&  Server::getRedirects() { return _redirects; }

const std::vector<Redirect>& Server::getRedirects() const { return _redirects; }

const std::map<std::string, std::string>& Server::getErrorPages() const { return _error_pages; }

// static void setup_default_error_pages(struct Config& cfg)
// {
// 	cfg.http.server.error_pages["404"] = "./resources/default_error_pages/404.html";
// 	cfg.http.server.error_pages["405"] = "./resources/default_error_pages/405.html";
// 	cfg.http.server.error_pages["500"] = "./resources/default_error_pages/50x.html";
// 	cfg.http.server.error_pages["501"] = "./resources/default_error_pages/50x.html";
// 	cfg.http.server.error_pages["502"] = "./resources/default_error_pages/50x.html";
// 	cfg.http.server.error_pages["503"] = "./resources/default_error_pages/50x.html";
// 	cfg.http.server.error_pages["504"] = "./resources/default_error_pages/50x.html";
// }

void Server::get_config(struct Config& cfg)
{
	ConfigCompiler::compileServer(*this, cfg);
}

void    Server::setPort(unsigned int port) { _port = port; }

void    Server::setHost(const std::string& hostname) { _hostname = hostname; }

void    Server::addLocation(const Location& new_location) { _locations.push_back(new_location); }

void    Server::addRedirect(std::string path, std::string redirect, StatusCode code)
{ 
    Redirect new_redirect;

    new_redirect._path = path;
    new_redirect._redirect = redirect;
    new_redirect._code = code;
    _redirects.push_back(new_redirect); 
}

void    Server::addErrorPage(std::string code, std::string html) { _error_pages[code] = html; }

HTTP::HTTP(): IBlock(HTTP_) {}

HTTP::HTTP(const HTTP& copy): IBlock(copy)
{
    _workers = copy._workers;
    _log_format = copy._log_format;
    for (size_t i = 0; i < copy._servers.size(); i++)
        _servers.push_back(copy._servers[i]);
}

HTTP& HTTP::operator=(const HTTP& copy)
{
    if (this != &copy)
    {
        IBlock::operator=(copy);
        _workers = copy._workers;
        _log_format = copy._log_format;
        _servers.clear();
        for (size_t i = 0; i < copy._servers.size(); i++)
            _servers.push_back(copy._servers[i]);
    }
    return *this;
}

HTTP::~HTTP() {};

std::string HTTP::getWorkers() const { return _workers; }

std::string HTTP::getLogFormat() const { return _log_format; }

std::vector<Server>& HTTP::getServers() { return _servers; }

const std::vector<Server>& HTTP::getServers() const { return _servers; }

void    HTTP::setWorkers(std::string workers) { _workers = workers; }

void    HTTP::setLogFormat(std::string log_format) { _log_format = log_format; }

void    HTTP::addServer(Server new_server) { _servers.push_back(new_server); }
