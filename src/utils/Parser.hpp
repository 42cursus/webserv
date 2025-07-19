/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:22:36 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 20:22:37 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include "webserv.hpp"

class Parser
{
	static Config make_default_config();

public:
	static Config parse(const char *filename);
};


#endif //PARSER_HPP
