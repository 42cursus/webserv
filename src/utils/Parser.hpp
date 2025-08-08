/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:22:36 by abelov            #+#    #+#             */
/*   Updated: 2025/08/08 16:57:17 by margo            ###   ########.fr       */
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
	ILLEGAL
} ;

typedef	struct s_token
{
	e_token	type;
	e_token next;
	int	line;
}	t_token;
class Parser
{
	private:
		std::string	_configRoot;
		Config	_config;
		t_token	_currentToken;
		t_token	_nextToken;
		IState*	_currentState;
		bool	_inBlock;
		std::vector<IBlock>	_blocks;
	
		Parser();
		Parser(const Parser& copy);
		Parser&	operator=(const Parser& copy);
		
	public:
		Parser(IState*	currentState);
		~Parser();
		
		IState*	getCurrentState() const;
		void	setCurrentState(IState& newState);
		bool 	isInBlock() const;
		std::string	getConfigRoot() const;
		Config	getConfig() const;
		
		void	toggle();
		
		static std::string read_file(const char *filename);
		static	std::map<std::string, std::string> init_mime_types();
		static Config parse(const char *filename);
		static Config make_default_config();
};


#endif //PARSER_HPP
