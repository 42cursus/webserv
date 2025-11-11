/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 19:27:13 by margo             #+#    #+#             */
/*   Updated: 2025/09/16 20:58:27 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//#include "webserv.hpp"
#include "Parser.hpp"

int strLenForConstChar(const char *str)
{
    int len = 0;

    for (int i = 0; str[i] != '\0'; i++)
        len++;

    return len;
}

char *strDupForConstChar(const char *str)
{
    int len = strLenForConstChar(str);
    char *newStr = new char[len + 1];

    for (int i = 0; i < len; i++)
        newStr[i] = str[i];
    newStr[len] = '\0';
    
    return newStr;
}