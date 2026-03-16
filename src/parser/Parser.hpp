/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 16:11:56 by margo             #+#    #+#             */
/*   Updated: 2026/02/01 17:44:45 by margo            ###   ########.fr       */
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
#include <climits>    // PATH_MAX
#include <unistd.h>   // readlink
#include <fstream>

#include "webserv.hpp"
#include "Location.hpp"
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
        ~Parser();

        // getters
        std::string getConfigRoot() const;
        unsigned int    getCurrentLine() const;
        t_token getCurrentToken() const;
        t_token getNextToken() const;
        std::vector<t_token>    getTokens() const;
        std::vector<t_token>::iterator  getCurrentIt() const;
        IBlock* getCurrentBlock() const;
        HTTP&   getConfig();
        const HTTP&   getConfig() const;
        Server& getLastServer();
        const Server& getLastServer() const;
        Location& getLastLocation();
        const Location& getLastLocation() const;
        CGI& getLastCGI();
        const CGI& getLastCGI() const;

        // setters
        void    setConfigRoot(std::string config_root);
        void    setCurrentToken(t_token current);
        void    setNextToken(t_token next);
        void    setCurrentIt(std::vector<t_token>::iterator current_it);
        void    setCurrentBlock(IBlock* current);
        
        // directive handlers
        void    handleWorkers(const std::vector<t_token> line);
        void    handleLogFormat(const std::vector<t_token> line);
        void    handleListen(const std::vector<t_token> line);
        void    handleName(const std::vector<t_token> line);
        void    handleRedirect(const std::vector<t_token> line);
        void    handleRoot(const std::vector<t_token> line);
        void    handleIndex(const std::vector<t_token> line);
        void    handleAutoIndex(const std::vector<t_token> line);
        void    handleMethods(const std::vector<t_token> line);
        void    handleMaxBodySize(const std::vector<t_token> line);
        void    handleExt(const std::vector<t_token> line);
        void    handleScript(const std::vector<t_token> line);
        void    handleCgiParam(const std::vector<t_token> line);
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
                Error(std::string msg);
                ~Error() throw();
                const char* what() const throw();
        } ;
    } ;

void	printTokens(std::vector<t_token> tokens);
    
#endif
