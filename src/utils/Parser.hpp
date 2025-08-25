/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:22:36 by abelov            #+#    #+#             */
/*   Updated: 2025/08/25 23:23:37 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <string>
//#include "webserv.hpp"
#include "State.hpp"
#include "../../include/webserv.hpp"



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
} ;

typedef	struct s_token
{
	e_token	type;
	std::string	literal;
	int	line;
	bool	operator==(const t_token& other) const;
}	t_token;
class Parser
{
private:
	std::string	_key;
	std::string	_configRoot;
	Config	_config;
	t_token	_currentToken;
	t_token	_nextToken;
	IState*	_currentState;
	bool	_inBlock;
	std::vector<IBlock*>	_blocks;
	std::string commentBuf;	

public:
	Parser();
	Parser(const Parser& copy);
	Parser&	operator=(const Parser& copy);
	Parser(IState*	currentState);
	~Parser();

	std::string getKey() const;
	void	setKey(std::string key);
	IState*	getCurrentState() const;
	void	setCurrentState(IState* newState);
	bool 	isInBlock() const;
	void	setInBlock(bool in);
	std::string	getConfigRoot() const;
	void	setConfigRoot(std::string configRoot);
	Config	getConfig() const;
	t_token	getCurrentToken() const;
	void	setCurrentToken(t_token token);
	t_token	getNextToken() const;
	void	setNextToken(t_token token);
	IBlock* getBlock(std::string key);
	void	addNewBlock(IBlock* newBlock);

	std::string	readUntil(std::string line, char delim);
	std::string readQuotedString(std::string word);
	void	toggle();

	static std::string read_file(const char *filename);
	t_token	makeToken(e_token key, std::string word, int linecount);
	std::vector<t_token> tokenize();
	Comment	makeComment(std::string buf, int line);
	static	std::map<std::string, std::string> init_mime_types();
	Config parse(std::vector<t_token>& tokens);
	void	parseServer();
	static Config make_default_config();
};


#endif //PARSER_HPP
