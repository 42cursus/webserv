/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 04:54:33 by abelov            #+#    #+#             */
/*   Updated: 2025/12/04 04:54:33 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <arpa/inet.h>
#include "webserv.hpp"
#include "State.hpp"
#include "Utils.hpp"
#include <stdint.h>

class Parser
{
public:
    enum	TokenType
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
    struct Token
    {
        TokenType	type;
        std::string	literal;
        int	line;
        bool	operator==(const Token& other) const;
    };
private:
    std::string	_key;
    std::string	_configRoot;
    Token	_currentToken;
    Token	_nextToken;
    std::vector<Parser::Token>::iterator _currentIt;
    IState*	_currentState;
    bool	_inBlock;
    std::vector<IBlock*>	_blocks;
    std::string commentBuf;

public:
    Parser();
    Parser(const Parser& copy);
    Parser(IState*	currentState);
    ~Parser();

    Config	_config;

    std::string getKey() const;
    void	setKey(std::string key);
    IState*	getCurrentState() const;
    void	setCurrentState(IState* newState);
    bool 	isInBlock() const;
    void	setInBlock(bool in);
    std::string	getConfigRoot() const;
    void	setConfigRoot(std::string configRoot);
    Config	getConfig() const;
    Token	getCurrentToken() const;
    void	setCurrentToken(Token token);
    Token	getNextToken() const;
    void	setNextToken(Token token);
    void	toggleCurrentToken();
    IBlock* getBlock(std::string key);
    void	addNewBlock(IBlock* newBlock);
    void    memset (void *s, int c, size_t n) __THROW;

    std::string	readUntil(std::string line, char delim);
    std::string	getFullLine(int line);
    std::string readQuotedString(std::string word);
    void	toggle();

    static std::string read_file(const char *filename);
    Token	makeToken(TokenType key, std::string word, int linecount);
    std::vector<Parser::Token> tokenize();
    Comment	makeComment(std::string buf, int line);
    static	std::map<std::string, std::string> init_mime_types();
    void	parse(std::vector<Parser::Token>& tokens);
    void	printTokens(std::vector<Parser::Token> tokens);
    void	parseServer(std::vector<Parser::Token>& tokens);
    static Config make_default_config();

    bool operator==(const Parser &rhs) const;

    bool operator!=(const Parser &rhs) const;

    class errorException: public std::exception
    {
    private:
        std::string _errorMsg;

    public:
        errorException(const std::string& msg);
        ~errorException() throw() {};
        const char*	what() const throw();
    };


};



#endif //CONFIGPARSER_HPP
