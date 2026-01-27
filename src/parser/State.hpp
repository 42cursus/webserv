/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   State.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 17:45:42 by margo             #+#    #+#             */
/*   Updated: 2026/01/27 18:06:49 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATE_HPP
#define STATE_HPP

#include <iostream>
#include <vector>
#include <map>

struct  CGI;
struct  Location;
struct Config;

enum    e_block_type
{
    HTTP_,
    SERVER_,
    LOCATION_,
    CGI_,
} ; 

class   IBlock
{
    private:
        bool    _in_block;
        e_block_type    _type;
        unsigned int    _line_start;
        unsigned int    _line_end;
        IBlock* _parent_block; // if NULL we're in main server block
        
        IBlock();
        
    public:
        IBlock(e_block_type type);
        IBlock(const IBlock& copy);
        IBlock& operator=(const IBlock& copy);
        bool    operator==(IBlock& oth);
        virtual ~IBlock();

        // getters
        bool    isInBlock() const;
        unsigned int    getStartLine() const;
        unsigned int    getEndLine() const;
        e_block_type    getBlockType() const;
        IBlock* getParent() const;
        
        // setters
        void    setInBlock(bool in_block);
        void    setStartLine(unsigned int line_start);
        void    setEndLine(unsigned int line_end);
        void    setParent(IBlock*   parent);
} ;

struct  CGI: public IBlock
{
    std::string _ext;
    std::string _script;
    std::map<std::string, std::string> _cgi_param; // std::map<key, value>
    CGI();
    CGI& operator=(const CGI& copy);
} ;

struct  Location: public IBlock
{  
    std::string _root; // where it's getting redirected;
    std::string _path; // the redirect 
    std::vector<std::string>    _index;
    std::vector<std::string>    _methods;
    unsigned long long   _max_body_size;
    bool    _autoindex;
    std::vector<CGI>    _cgi;
    Location();
    Location&   operator=(const Location& copy);
} ;

class   Server: public IBlock
{
    private:
        unsigned int _port;
        std::string _hostname;
        std::vector<Location>   _locations;
        std::map<std::string, std::string> _error_pages; // map<error code, path to html>

    public:
        Server();
        Server(const Server& copy);
        Server& operator=(const Server& copy);
        ~Server();
        
        unsigned int getPort() const;
        std::string getHost() const;
        std::vector<Location>& getLocations();
        const std::vector<Location>& getLocations() const;
        const std::map<std::string, std::string>& getErrorPages() const;

        void    setPort(unsigned int port);
        void    setHost(std::string hostname);
        void    addLocation(Location new_location);
        void    addErrorPage(std::string code, std::string html);
		void	get_config(struct Config &cfg);
} ;

class   HTTP: public IBlock
{
    private:
        std::string _workers;
        std::string _log_format;
        std::vector<Server> _servers;
    
    public:
        HTTP();
        HTTP(const HTTP& copy);
        HTTP& operator=(const HTTP& copy);
        ~HTTP();

        std::string getWorkers() const;
        std::string getLogFormat() const;
        std::vector<Server>& getServers();
        const std::vector<Server>& getServers() const;
        
        void    setWorkers(std::string workers);
        void    setLogFormat(std::string log_format);
        void    addServer(Server new_server);

        void    printConfig();
} ;

#endif
