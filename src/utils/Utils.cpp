/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/16 19:27:13 by margo             #+#    #+#             */
/*   Updated: 2025/10/01 23:37:18 by margo            ###   ########.fr       */
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

int strnCmp(const char *s1, const char *s2, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (s1[i] != s2[i])
            return s1[i] - s2[i];
    }

    return 0;
}

void    *memSet(void *s, int c, size_t n)
{
    unsigned char *ptr;

    ptr = (unsigned char *)s;
    for (size_t i = 0; i < n; i++)
        ptr[i] = c;

    return (s);
}

/*
std::string Parser::getFullLine(int line)
{
    
}
*/