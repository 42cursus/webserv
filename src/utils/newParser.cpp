/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newParser.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 21:52:43 by margo             #+#    #+#             */
/*   Updated: 2026/01/21 22:06:14 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "newParser.hpp"

t_token Parser::makeToken(e_token key, std::string word, int linecount)
{
    t_token token;

    token.type = key;
    token.literal = word;
    token.line = linecount;

    return token;
}

void    Parser::handleDirective()
{
    _current_line++;
    
}

void    Parser::handleBlockIn()
{
    _in_block = true;

}

void    Parser::hangleBlockOut()
{
    _in_block = false;
}

e_line_type Parser::checkLineType(std::string line)
{
    if (line.empty())
        return EMPTY;
    else if (line.back('{'))
        return BLOCK_IN;
    else if (line.back('}'))
        return BLOCK_OUT;
    else if (line.back(';'))
        return DIRECTIVE;
    else
        return ERROR;
}

void    Parser::tokenise()
{
    std::string line;
    std::ifstream file(_config_root.c_str());
    
    _current_line = 0;
    try {
        file.is_open();
    }
    catch (std::exception&  e) {
        std::cerr << e.what() << std::endl;
    }
    
    while (std::getline(file, line))
    {
        std::istringstream  iss(line);
        std::string word;

        _current_line_type = checkLineType(line);
        if (_current_line_type == EMPTY)
            continue ;
        else if (_current_line_type == BLOCK_IN)
        {

        }
        else if (_current_line_type == BLOCK_OUT)
        {

        }
        else if (_current_line_type == DIRECTIVE)
        {

        }
    }
}