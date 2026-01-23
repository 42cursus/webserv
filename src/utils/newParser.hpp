/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newParser.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 16:11:56 by margo             #+#    #+#             */
/*   Updated: 2026/01/23 12:55:57 by fsmyth           ###   ########.fr       */
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
#include <sstream>
#include <fstream>
#include "newState.hpp"

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
class   Parser
{
    private:    
        std::string _config_root;
        unsigned int _current_line;
        t_token _current;
        t_token _next;
        std::vector<t_token>    _tokens;
        std::vector<t_token>::iterator  _current_it;
        std::vector<Server> _servers;
        bool    _in_block;
        e_line_type _current_line_type;
        IBlock* _current_block;
        
        Parser();
        Parser(const Parser& copy);
        Parser& operator=(const Parser& copy);
        
    public:
        Parser(std::string filePath): _config_root(filePath) {};
        ~Parser() {};
        // getters
        std::string getConfigRoot() const { return _config_root; };
        unsigned int    getCurrentLine() const { return _current_line; };
        t_token getCurrentToken() const { return _current; };
        t_token getNextToken() const { return _next; };
        std::vector<t_token>    getTokens() const { return _tokens; };
        std::vector<t_token>::iterator  getCurrentIt() const { return _current_it; };
        std::vector<Server> getServers() const { return _servers; };
        IBlock* getCurrentBlock() const { return _current_block; };
        
        // setters
        void    setConfigRoot(std::string config_root) { _config_root = config_root; };
        void    setCurrentToken(t_token current) { _current = current; };
        void    setNextToken(t_token next) { _next = next; };
        void    setCurrentIt(std::vector<t_token>::iterator current_it) { _current_it = current_it; };
        void    setCurrentBlock(IBlock* current) { _current_block = current; };
        
        // main loop
        void    tokenise();
        void    toggle();
        void    parse();

        // utils
        e_line_type checkLineType(std::string line);
        t_token makeToken(e_token   key, std::string word, int linecount);
        void    handleDirective();
        void    handleBlockIn();
        void    hangleBlockOut();

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
