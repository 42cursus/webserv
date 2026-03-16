/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 17:01:05 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 17:01:05 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigValidator.hpp"
#include "Parser.hpp"
#include <climits>
#include <cstdlib>
#include <cctype>

bool ConfigValidator::validateCgiParam(std::string cgi_param)
{
	return !cgi_param.empty() && cgi_param[0] == '$';
}

bool ConfigValidator::validateMethod(std::string method)
{
	return method == "PUT" || method == "GET" || method == "DELETE" || method == "POST";
}

bool ConfigValidator::validateErrorCode(std::string error_code)
{
	if (error_code.size() > 10 || std::atol(error_code.c_str()) > INT_MAX || std::atol(error_code.c_str()) < 0)
		throw Parser::Error("Error: syntax error: invalid error_code");

	for (int i = 0; error_code[i]; i++) {
		if (!std::isdigit(error_code[i]))
			throw Parser::Error("Error: syntax error: invalid error_code");
	}
	return (std::atol(error_code.c_str()) >= 100 && std::atol(error_code.c_str()) <= 599);
}

bool ConfigValidator::validateErrorPage(std::string error_page)
{
	size_t ext = error_page.find(".html");

	if (ext == std::string::npos)
		return false;
	if (error_page.substr(ext) != ".html")
		return false;
	return true;
}

bool ConfigValidator::validateBool(std::string boolean)
{
	return boolean == "true" || boolean == "false";
}

bool ConfigValidator::validateCgiScriptExt(std::string ext, std::string script)
{
	size_t find = script.find(ext);

	if (find == std::string::npos)
		return false;
	if (script.substr(find) != ext)
		return false;
	return true;
}

bool ConfigValidator::validateLocationRedirect(std::string path, std::string root)
{
	if (path.empty() || root.empty())
		return false;
	char path_end = path[path.length() - 1];
	char root_end = root[root.length() - 1];

	if ((path_end == '/' && root_end != '/') || (root_end == '/' && path_end != '/'))
		return false;
	return true;
}
