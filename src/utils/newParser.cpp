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

void    Parser::tokenise()
{
    std::string line;
    std::ifstream file(_config_root.c_str());
    
    _current_line = 0;
}