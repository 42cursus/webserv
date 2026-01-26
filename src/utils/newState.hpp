/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newState.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 17:45:42 by margo             #+#    #+#             */
/*   Updated: 2026/01/25 20:11:56 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NEWSTATE_HPP
#define NEWSTATE_HPP

#include <iostream>
#include <vector>
#include <map>
#include "newParser.hpp"

enum    e_block_type
{
    HTTP_,
    SERVER_,
    LOCATION_,
    CGI_,
} ; 

struct  Directive
{
    std::string _key; // left side
    std::vector<std::string> _parameter; // right side;
    //unsigned  int   _equal; // = index
    unsigned  int   _line;
    Directive& operator=(const Directive& other);    
} ;

class   IBlock
{
    private:
        bool    _in_block;
        unsigned int    _line_start;
        unsigned int    _line_end;
        e_block_type    _type;
        IBlock* _parent_block; // if NULL we're in main server block
        std::vector<Directive> _directives;
        
        IBlock();
        
    public:
        IBlock(e_block_type type);
        IBlock(const IBlock& copy);
        IBlock& operator=(const IBlock& copy);
        bool    operator==(IBlock& oth);
        ~IBlock();

        // getters
        bool    isInBlock() const;
        unsigned int    getStartLine() const;
        unsigned int    getEndLine() const;
        e_block_type    getBlockType() const;
        IBlock* getParent() const;
        std::vector<Directive>  getDirectives() const;
        
        // setters
        void    setInBlock(bool in_block);
        void    setStartLine(unsigned int line_start);
        void    setEndLine(unsigned int line_end);
        void    setParent(IBlock*   parent);
        void    addDirective(Directive new_directive);
        
        // virtual void    start(Parser& parser) = 0;
        // virtual void    toggle(Parser& parser) = 0;
        // virtual void    exit(Parser& parser) = 0;
} ;

struct  CGI: public IBlock
{
    std::string _ext;
    std::string _script;
    CGI();
    CGI& operator=(const CGI& copy);
    // void    start(Parser& parser);
    // void    toggle(Parser& parser);
    // void    exit(Parser& parser);
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
    // void    start(Parser& parser);
    // void    toggle(Parser& parser);
    // void    exit(Parser& parser);
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
        std::vector<Location>   getLocations() const;
        std::map<std::string, std::string>  getErrorPages() const;

        void    setPort(unsigned int port);
        void    setHost(std::string hostname);
        void    addLocation(Location& new_location);
        void    addErrorPage(std::string code, std::string html);
        
        //void    start(Parser& parser);
        // void    toggle(Parser& parser);
        // void    exit(Parser& parser);
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
        std::vector<Server> getServers() const;
        
        void    setWorkers(std::string workers);
        void    setLogFormat(std::string log_format);
        void    addServer(Server new_server);
        
        // void    start(Parser& parser);
        // void    toggle(Parser&  parser);
        // void    exit(Parser& parser);
} ;

#endif