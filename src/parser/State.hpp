/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   State.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mganchev <mganchev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 17:45:42 by margo             #+#    #+#             */
/*   Updated: 2026/03/18 12:31:14 by mganchev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATE_HPP
#define STATE_HPP

#include <cstddef>
#include <iostream>
#include <map>
#include <vector>
#include "StatusCode.hpp"

struct CGI;
struct Location;
struct Config;

enum e_block_type {
    HTTP_,
    SERVER_,
    LOCATION_,
    CGI_,
};

struct TrieNode;

class IBlock {
    private:
        bool    _in_block;
        e_block_type    _type;
        unsigned int    _line_start;
        unsigned int    _line_end;
        IBlock* _parent_block; // if NULL we're in main server block

    protected:
        IBlock();

    public:
        IBlock(e_block_type type);
        IBlock(const IBlock& copy);
        IBlock& operator=(const IBlock& copy);
        bool    operator==(IBlock& oth);
        virtual ~IBlock();

        std::string getName() const { return ""; };
        std::string getCode() const { return ""; };

        // getters
        bool isInBlock() const;
        unsigned int getStartLine() const;
        unsigned int getEndLine() const;
        e_block_type getBlockType() const;
        IBlock *getParent() const;

        // setters
        void setInBlock(bool in_block);
        void setStartLine(unsigned int line_start);
        void setEndLine(unsigned int line_end);
        void setParent(IBlock *parent);
};

struct  Redirect
{
    std::string _path;
    std::string _redirect;
    StatusCode _code;
    Redirect &operator=(const Redirect &copy);
} ;

std::ostream& operator<<(std::ostream& os, const Redirect& redirect);

struct CGI : public IBlock {
    std::string _ext;
    std::string _script;
    std::string _cgi_pass;
    std::map<std::string, std::string> _cgi_param;// std::map<key, value>
    CGI();
    CGI &operator=(const CGI &copy);
};

std::ostream& operator<<(std::ostream& os, const CGI& cgi);

class Server : public IBlock {
    private:
        unsigned int _port;
        std::string _hostname;
        std::vector<Location> _locations;
        std::vector<Redirect>   _redirects;
        std::map<std::string, std::string> _error_pages;// map<error code, path to html>

    public:
        Server();
        Server(const Server &copy);
        Server &operator=(const Server &copy);
        ~Server();

        unsigned int getPort() const;
        std::string getHost() const;
        std::vector<Location> &getLocations();
        const std::vector<Location> &getLocations() const;
        std::vector<Redirect> &getRedirects();
        const std::vector<Redirect> &getRedirects() const;
        const std::map<std::string, std::string> &getErrorPages() const;

        void setPort(unsigned int port);
        void setHost(const std::string& hostname);
        void addLocation(const Location& new_location);
        void addRedirect(std::string path, std::string redirect, StatusCode code);
        void addErrorPage(std::string code, std::string html);
        void get_config(struct Config &cfg);
};

class HTTP : public IBlock {
    private:
        std::string _workers;
        std::string _log_format;
        std::vector<Server> _servers;

    public:
        HTTP();
        HTTP(const HTTP &copy);
        HTTP &operator=(const HTTP &copy);
        ~HTTP();

        std::string getWorkers() const;
        std::string getLogFormat() const;
        std::vector<Server> &getServers();
        const std::vector<Server> &getServers() const;

        void setWorkers(std::string workers);
        void setLogFormat(std::string log_format);
        void addServer(Server new_server);

        void printConfig();
};

#endif
