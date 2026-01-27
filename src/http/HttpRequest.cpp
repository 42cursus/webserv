/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:41:03 by abelov            #+#    #+#             */
/*   Updated: 2026/01/23 15:52:59 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Prefix.hpp"
#include "webserv.hpp"
// #include "src/cgi/CgiHandler.hpp"
// #include "LocationConfig.hpp"

HttpRequest::HttpRequest(const std::string &path) : path(path)
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
		std::cerr << "Failed to parse start line." << std::endl;
		throw HttpRequest::GenericException();
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
			std::cerr << "Malformed request in header: " << FT_BLUE << line.substr(0, line_end) << FT_RESET << std::endl;
			throw HttpRequest::GenericException();
		}
		field += std::tolower(line[i++]);
	}
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


static bool ends_with(const std::string &s, const std::string &suffix) {
	if (suffix.size() > s.size())
		return false;
	const size_t offset = s.size() - suffix.size();
	return s.compare(offset, suffix.size(), suffix) == 0;
}

std::string
HttpRequest::getHtmlResponse(const Config &conf, HttpResponse& res)
{
	Location *location = loc_trie_search(conf.http.server.loc_trie, path);
	std::basic_string<char> filename = path.substr(1, path.length());
	// std::cout << FT_BOLD << FT_RED << path << FT_RESET << std::endl;

	if (filename.empty())
	{
		if (!location->_autoindex)
			filename = location->_index[0];
		else
			; // DO AUTOINDEX FUNCTION
	}

	// LocationConfig lc(*location);
	res.filename = filename;
	std::string output;
	if (ends_with(filename, ".bla"))
	{
	// 	// CgiHandler handler(*this, lc, filename, res);
	// 	res.statuscode = itoa(handler.do_run());
	// 	output = handler.raw_output();
	}
	else if (filename == "teapot")
	{
		res.statuscode = "418";
		res.statusmsg = "I'm a Teapot";
		output = "{\"msg\" = \"I'm a Teapot\"}";
		res.headers["content-type"] = "application/json";
	}
	else
	{
		res.headers["content-type"] = getMimeType(filename);
		output = readHtmlFile(filename, conf);
	}
	return output;
}

std::string
HttpRequest::readHtmlFile(const std::string &filename, const Config &conf)
{
	const std::string &root_folder = conf.http.server.locations[0]->_root;

	std::string filePath = root_folder + "/" + filename;
	std::ifstream file(filePath.c_str(), std::ios_base::in);

	if (!file) {
		std::cerr << "File not found." << std::endl;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	return buffer.str();
}

std::string HttpRequest::getMimeType(const std::string &path)
{
	std::map<std::string, std::string> mimeTypes;

	mimeTypes.insert(std::make_pair("html", "text/html"));
	mimeTypes.insert(std::make_pair("htm", "text/html"));
	mimeTypes.insert(std::make_pair("css", "text/css"));
	mimeTypes.insert(std::make_pair("js", "text/javascript"));
	mimeTypes.insert(std::make_pair("jpeg", "image/jpeg"));
	mimeTypes.insert(std::make_pair("jpg", "image/jpeg"));
	mimeTypes.insert(std::make_pair("png", "image/png"));
	mimeTypes.insert(std::make_pair("mp4", "video/mp4"));

	std::string fileExtension = path.substr(path.find_last_of(".") + 1);

	return mimeTypes[fileExtension];
}


const char *HttpRequest::GenericException::what() const throw()
{
	return "Client exception happened";
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
