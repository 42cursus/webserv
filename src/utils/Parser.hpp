/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mganchev <mganchev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 16:11:56 by margo             #+#    #+#             */
/*   Updated: 2026/01/27 13:21:30 by mganchev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
	- splitting line based on = sing;
	- we have 3 types of lines - ending in BLOCK_START, BLOCK_END or SEMICOLON
	- txt file or map container of KEYS + enum/function that checks left half of equal sign
	- map of error pages;
*/

#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "../../include/webserv.hpp"
#include "State.hpp"

class  IBlock;
class  Server;
class  HTTP;
struct Location;
struct CGI;

enum    e_line_type
{
    EMPTY, // empty line
    DIRECTIVE, // semicolon
    BLOCK_IN, // { > beginning of block
    BLOCK_OUT, // } > end of block
    ERROR
} ;

enum	e_token
{
    EQUAL,
	KEY,
	QUOTES,
	BLOCK_START,
	BLOCK_END,
	SEMICOLON,
	REGEX,
	ILLEGAL,
	NONE,
};

typedef	struct s_token
{
	e_token	type;
	std::string	literal;
	unsigned int	line;
	bool	operator==(const s_token& other) const;
}	t_token;

class   Parser
{
    private:    
        std::string _config_root;
        unsigned int _current_line;
        t_token _current;
        t_token _next;
        std::vector<std::string>  _key_database;
        typedef void (Parser::*Directive_handler)(const std::vector<t_token>);
        std::map<std::string, Directive_handler> _directive_handlers;
        std::vector<t_token>    _tokens;
        std::vector<t_token>::iterator  _current_it;
        e_line_type _current_line_type;
        IBlock* _current_block;
        HTTP    _config;
        
        Parser();
        Parser(const Parser& copy);
        Parser& operator=(const Parser& copy);
        
    public:
        Parser(std::string filePath);
        ~Parser() {};

        // getters
        std::string getConfigRoot() const { return _config_root; };
        unsigned int    getCurrentLine() const { return _current_line; };
        t_token getCurrentToken() const { return _current; };
        t_token getNextToken() const { return _next; };
        std::vector<t_token>    getTokens() const { return _tokens; };
        std::vector<t_token>::iterator  getCurrentIt() const { return _current_it; };
        IBlock* getCurrentBlock() const { return _current_block; };
        HTTP&   getConfig() { return _config; };
        const HTTP&   getConfig() const { return _config; };
        Server& getLastServer() { return _config.getServers().back(); }
        const Server& getLastServer() const { return _config.getServers().back(); }
        Location& getLastLocation() { return getLastServer().getLocations().back(); }
        const Location& getLastLocation() const { return getLastServer().getLocations().back(); }
        CGI& getLastCGI() { return getLastLocation()._cgi.back(); }
        const CGI& getLastCGI() const { return getLastLocation()._cgi.back(); }

        // setters
        void    setConfigRoot(std::string config_root) { _config_root = config_root; };
        void    setCurrentToken(t_token current) { _current = current; };
        void    setNextToken(t_token next) { _next = next; };
        void    setCurrentIt(std::vector<t_token>::iterator current_it) { _current_it = current_it; };
        void    setCurrentBlock(IBlock* current) { _current_block = current; };
        
        // directive handlers
        void    handleWorkers(const std::vector<t_token> line);
        void    handleLogFormat(const std::vector<t_token> line);
        void    handleListen(const std::vector<t_token> line);
        void    handleName(const std::vector<t_token> line);
        void    handleRoot(const std::vector<t_token> line);
        void    handleIndex(const std::vector<t_token> line);
        void    handleAutoIndex(const std::vector<t_token> line);
        void    handleMethods(const std::vector<t_token> line);
        void    handleMaxBodySize(const std::vector<t_token> line);
        void    handleExt(const std::vector<t_token> line);
        void    handleScript(const std::vector<t_token> line);
        void    handleErrorPage(const std::vector<t_token> line);

        // main loop
        void    init_key_database();
        void    init_directive_handlers();
        void    init_parser();
        void    tokenise();
        void    parse();

        // utils
        e_line_type checkLineType(std::vector<t_token> line);
        std::string    findKeyInDatabase(std::string key);
        t_token makeToken(e_token   key, std::string word, int linecount);
        std::vector<t_token>::const_iterator getTokenFromVector(const std::vector<t_token>& vec, e_token key);
        std::string readQuotedString(std::istringstream& iss);
        void    readLogFormatString(std::ifstream& file, std::istringstream& iss, std::string& word);
        void    handleDirective(std::vector<t_token> line);
        void    handleBlockIn(std::vector<t_token> line);
        void    handleBlockOut();

        // exceptions
        class   Error: public std::exception
        {
            private:
                std::string _msg;
            
            public:
                Error(std::string msg) { _msg = msg; };
                ~Error() throw() {};
                const char* what() const throw() { return _msg.c_str(); };
        } ;
    } ;

#endif
