/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:41:03 by abelov            #+#    #+#             */
/*   Updated: 2026/02/03 00:35:06 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Prefix_suffix.hpp"
#include "State.hpp"
#include "webserv.hpp"
#include "src/handlers/CgiHandler.hpp"
#include "Location.hpp"

HttpRequest::HttpRequest(const std::string &path) : path(path), content_length(0)
{}

HttpRequest::~HttpRequest()
{}

size_t	HttpRequest::_parseStartLine(const std::string &line)
{
	int							i = 0;
	int							line_end = line.find("\r\n");
	std::vector<std::string>	tokens;
	// std:

	while (i < line_end)
	{
		std::string	token;
		while (line[i] != ' ' && i < line_end)
			token += line[i++];
		tokens.push_back(token);
		while (line[i] == ' ' && i < line_end)
			i++;
	}
	if (tokens.size() != 3)
	{
		throw HttpRequest::MalformedStartlineException();
	}
	method = tokens[0];
	headers["method"] = method;
	path = tokens[1];
	headers["path"] = path;
	protocol = tokens[2];
	headers["protocol"] = protocol;
	return (line_end + 2);
}

size_t	HttpRequest::_parseHeader(const std::string &line)
{
	int			i = 0;
	int			line_end = line.find("\r\n");
	std::string	field;
	std::string	content;

	while (i < line_end)
	{
		if (line[i] == ':')
			break;
		if (line[i] == ' ')
		{
			// std::cerr << "Malformed request in header: " << FT_BLUE << line.substr(0, line_end) << FT_RESET << std::endl;
			throw HttpRequest::MalformedHeaderException();
		}
		field += std::tolower(line[i++]);
	}
	if (i == line_end)
		throw HttpRequest::GenericException();
	while (line[++i] == ' ')
		;
	while (i < line_end)
		content += line[i++];
	headers[field] = content;
	return (line_end + 2);
}

void HttpRequest::parseRequest(const std::string &rawRequest)
{
	std::string	line;
	int			line_end;

	header_len = rawRequest.length();
	line = &rawRequest[_parseStartLine(rawRequest)];
	line_end = line.find("\r\n");
	// std::cout << "line_end: " << line_end << std::endl;
	while (line_end != 0)
	{
		line = &line[_parseHeader(line)];
		line_end = line.find("\r\n");
		// std::cout << "line_end: " << line_end << std::endl;
	}
}


// static bool ends_with(const std::string &s, const std::string &suffix) {
// 	if (suffix.size() > s.size())
// 		return false;
// 	const size_t offset = s.size() - suffix.size();
// 	return s.compare(offset, suffix.size(), suffix) == 0;
// }

bool		HttpRequest::is_method_permitted(Location *location) const
{
	return std::find(
		location->_methods.begin(),
		location->_methods.end(),
		this->method
	) != location->_methods.end();
}

static bool directory_exists(HttpResponse& res)
{
	std::string path = res.location->_root + res.filename;

	return !access(path.c_str(), F_OK);
}

StatusCode HttpRequest::getHtmlResponse(HttpResponse& res, Location *location)
{
	StatusCode	status = SC_200;

	// std::cout << FT_BOLD << FT_RED << path << FT_RESET << std::endl;

	if (res.filename.empty() || *res.filename.rbegin() == '/')
	{
		if (!directory_exists(res))
			throw HttpResponse::Exception404();

		std::string							path;
		std::vector<std::string>::iterator	it = res.location->_index.begin();
		for (; it != res.location->_index.end(); it++)
		{
			path = res.location->_root + res.filename + *it;
			// std::cout << path << std::endl;
			if (access(path.c_str(), F_OK) == 0)
				break ;
		}
		if (it == res.location->_index.end())
		{
			if (!res.location->_autoindex)
				throw HttpResponse::Exception403();
			res.buildAutoindexBody(); // DO AUTOINDEX FUNCTION
			res.headers["content-type"] = "text/html";
			return SC_200;
		}
		res.filename = res.filename + *it;
	}

	if (res.filename == "teapot")
	{
		status = SC_418;
		res.body = "{\"msg\": \"I'm a Teapot\"}";
		res.headers["content-type"] = "application/json";
	}
	else
	{
		res.headers["content-type"] = getMimeType(res.filename);
		status = res.readHtmlFile(res.location->_root + res.filename);
	}
	return status;
    (void)location;
}

std::string HttpRequest::getMimeType(const std::string &path) const {
	std::map<std::string, std::string> mimeTypes;

	mimeTypes.insert(std::make_pair("html", "text/html"));
	mimeTypes.insert(std::make_pair("htm", "text/html"));
	mimeTypes.insert(std::make_pair("css", "text/css"));
	mimeTypes.insert(std::make_pair("js", "text/javascript"));
	mimeTypes.insert(std::make_pair("jpeg", "image/jpeg"));
	mimeTypes.insert(std::make_pair("jpg", "image/jpeg"));
	mimeTypes.insert(std::make_pair("png", "image/png"));
	mimeTypes.insert(std::make_pair("ico", "image/x-icon"));
	mimeTypes.insert(std::make_pair("mp4", "video/mp4"));

	std::string fileExtension = path.substr(path.find_last_of(".") + 1);

	return mimeTypes[fileExtension];
}

HttpRequest::e_method	HttpRequest::get_method() const
{
	if (this->method == "GET")
		return GET;
	if (this->method == "POST")
		return POST;
	if (this->method == "PUT")
		return PUT;
	if (this->method == "DELETE")
		return DELETE;
	throw GenericException();
}

const char *HttpRequest::GenericException::what() const throw()
{
	return "Client exception happened";
}

const char *HttpRequest::MalformedStartlineException::what() const throw()
{
	return "Malformed request in start line";
}

const char *HttpRequest::MalformedHeaderException::what() const throw()
{
	return "Malformed request in header";
}

HttpRequest::HttpRequest()
{

}

void	HttpRequest::printBody(/* ... */) const
{
	size_t i;
	for (i = 0; i < this->body.size() && i < MAX_BODY_SIZE; i++)
	{
		char c = this->body[i];
		if (std::isprint(c))
			// std::cout << "\e[32m" << std::setw(2) << c << ' ';
			std::cout << FT_GREEN << c;
		else
			std::cout << FT_RED << ' ' << std::hex << std::setw(2) << std::setfill(' ') << std::setfill('0') << (int)(u_char)c << ' ';
	}
	if (i < this->body.size())
		std::cout << FT_BOLD << FT_BLUE << " [...]";
	std::cout << FT_RESET << std::setbase(DECIMAL_BASE) << std::endl << std::endl;
}
