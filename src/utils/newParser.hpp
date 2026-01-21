/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newParser.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 16:11:56 by margo             #+#    #+#             */
/*   Updated: 2026/01/21 17:52:12 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
	- splitting line based on = sing;
	- we have 3 types of lines - ending in BLOCK_START, BLOCK_END or SEMICOLON
	- txt file or map container of KEYS + enum/function that checks left half of equal sign
	- map of error pages;
*/

#ifndef NEWPARSER_HPP
#define NEWPARSER_HPP

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdbool>
#include <map>

enum    e_line_type
{
    EMPTY, // empty line
    DIRECTIVE, // semicolon
    BLOCK_IN, // { > beginning of block
    BLOCK_OUT // } > end of block
} ;


enum	e_token
{
	EOL,
	KEY,
	VAR,
	QUOTES,
	BLOCK_START,
	BLOCK_END,
	SEMICOLON,
	REGEX,
	COMMENT,
	ILLEGAL,
	NONE,
};

typedef	struct s_token
{
	e_token	type;
	std::string	literal;
	int	line;
	//bool	operator==(const s_token& other) const;
}	t_token;

struct  CGI
{
    std::string _ext;
    std::string _script;
} ;

struct  Location
{  
    std::string _root; // where it's getting redirected;
    std::string _path; // the redirect 
    std::vector<std::string>    _index;
    std::vector<std::string>    _methods;
    uint64_t   _max_body_size;
    bool    _autoindex;
    std::vector<CGI>    _cgi;
} ;

class   Server
{
    private:
        unsigned int _port;
        std::string _hostname;
        std::vector<Location>   _locations;
        std::map<std::string, std::string> _error_pages; // map<error code, path to html>

    public:
        
} ;

class   Parser
{
    private:    
        std::string _config_root;
        t_token _current;
        t_token _next;
        std::vector<t_token>    _tokens;
        std::vector<t_token>::iterator  _current_it;
        std::vector<Server> _servers;
        bool    _in_block;
        e_line_type _current_line; 
        
        Parser();
        Parser(const Parser& copy);
        Parser& operator=(const Parser& copy);
        ~Parser();
    public:

        // getters
        std::string getConfigRoot() const { return _config_root; };
        t_token getCurrentToken() const { return _current; };
        t_token getNextToken() const { return _next; };
        std::vector<t_token>    getTokens() const { return _tokens; };
        std::vector<t_token>::iterator  getCurrentIt() const { return _current_it; };
        std::vector<Server> getServers() const { return _servers; };
        
        // setters
        void    setConfigRoot(std::string config_root) { _config_root = config_root; };
        void    setCurrentToken(t_token current) { _current = current; };
        void    setNextToken(t_token next) { _next = next; };
        void    setCurrentIt(std::vector<t_token>::iterator current_it) { _current_it = current_it; };
} ;

#endif