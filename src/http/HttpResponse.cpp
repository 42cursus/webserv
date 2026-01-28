/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:40:56 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 21:40:57 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sstream>
#include <fstream>
#include <iostream>
#include "HttpResponse.hpp"
#include "webserv.hpp"
#include "Location.hpp"

std::string itoa(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string HttpResponse::readHtmlFile(const std::string &filename, const Location *location)
{
	const std::string &root_folder = location->_root;

	std::string filePath = root_folder + filename;
	std::ifstream file(filePath.c_str(), std::ios_base::in);

	if (!file) {
		std::cerr << "File not found." << std::endl;
		this->statuscode = "404";
		this->statusmsg = "Not Found";
		throw GenericException();
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	return buffer.str();
}

void HttpResponse::buildHttpResponse(void)
{
	std::ostringstream buffer;

	// headers["content-type"] = mimetype;
	// headers["content-length"] = itoa(body.length());
	buffer << "HTTP/1.1 " << statuscode << " " << statusmsg << "\r\n";

	for (StringMap::const_iterator it = headers.begin(); it != headers.end(); ++it)
		buffer << it->first << ": " << it->second << "\r\n";
	buffer << "\r\n" << body;
	response = buffer.str();
}

HttpResponse::HttpResponse() : start(0)
{

}


const char *HttpResponse::GenericException::what() const throw()
{
	return "Client exception happened";
}
