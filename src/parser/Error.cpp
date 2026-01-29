/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mganchev <mganchev@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 08:56:33 by mganchev          #+#    #+#             */
/*   Updated: 2026/01/29 09:57:18 by mganchev         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "State.hpp"

Parser::Error::Error(std::string msg) { _msg = msg; }

Parser::Error::~Error() throw() {}

const char* Parser::Error::what() const throw()
{
    return _msg.c_str();
}

bool    Parser::validateCgiParam(std::string   cgi_param)
{
    return cgi_param[0] == '$';
}

bool    Parser::validateMethod(std::string  method)
{
    return method == "PUT" || method == "GET" || method == "DELETE" || method == "POST";
}

bool    Parser::validateErrorPage(std::string   error_page)
{
    size_t ext = error_page.find(".html");

    if (ext == std::string::npos)
        return false;
    if (error_page.substr(ext) != ".html")
        return false;
    
    return  true;
}

bool    Parser::validateBool(std::string    boolean)
{
    return boolean == "true" || boolean == "false";
}

bool    Parser::validateCgiScriptExt(std::string  ext, std::string script)
{
    size_t  find = script.find(ext);

    if (find == std::string::npos)
        return false;
    if (script.substr(find) != ext)
        return false;

    return true;
}

bool    Parser::validateLocationRedirect(std::string path, std::string root)
{
    char    path_end = path[path.length() - 1];
    char    root_end = root[root.length() - 1];

    if ((path_end == '/' && root_end != '/') || (root_end == '/' && path_end != '/'))
        return false;
    
    return true;
}