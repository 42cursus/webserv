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

#include <ctime>
#include <sstream>
#include <fstream>
#include <iostream>
#include "HttpResponse.hpp"
#include "webserv.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include "Location.hpp"

std::string itoa(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

StatusCode HttpResponse::readHtmlFile(const std::string &filename)
{
	StatusCode status = SC_200;
	std::ifstream file(filename.c_str(), std::ios_base::in);

	if (!file) {
		std::cerr << "File not found." << std::endl;
		throw Exception404();
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	this->body = buffer.str();
	return status;
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

static std::vector<std::string> _get_directory_members(std::string path)
{
	std::vector<std::string>	filenames;
	struct dirent				*dirent;
	DIR							*dir;

	dir = opendir(path.c_str());
	dirent = readdir(dir);
	for (dirent = readdir(dir); dirent != NULL; dirent = readdir(dir))
		filenames.push_back(dirent->d_name);

	closedir(dir);
	return (filenames);
}

static std::string timespec_to_str(struct timespec& ts)
{
	char	buf[64];
	std::tm* time = std::localtime(&ts.tv_sec);

	std::strftime(buf, 64, "%Y-%b-%d %H:%M", time);

	return buf;
}

void HttpResponse::buildAutoindexBody(void)
{
	std::vector<std::string>	filenames = _get_directory_members(location->_root);
	std::vector<std::string>::iterator	it = filenames.begin();
	struct stat	statbuf;

	this->body += "<html>\n<head><title>Index of " + location->_path + "</title></head>\n";
	this->body += "<body>\n<h1>Index of " + location->_path + "</h1><hr><pre>";
	
	for (; it != filenames.end(); it++)
	{
		std::string path = location->_root + *it;
		std::string line;
		std::string size;

		stat(path.c_str(), &statbuf);
		if (statbuf.st_mode & S_IFDIR)
		{
			*it += '/';
			size = "-";
		}
		else
		{
			size = ::itoa(statbuf.st_size);
		}
		line += "<a href=\"" + *it + "\">" + *it + "</a>";
		for (size_t n = 100; n > line.length(); n--)
			line += ' ';
		line += timespec_to_str(statbuf.st_mtim);
		line += "    " + size;
		line += '\n';
		this->body += line;
	}
	this->body += "</pre><hr></body>\n</html>\n";
}

HttpResponse::HttpResponse() : start(0)
{

}

const char *HttpResponse::GenericException::what() const throw()
{
	return "Client exception happened";
}

const char *HttpResponse::Exception404::what() const throw()
{
	return "File Not Found";
}

void HttpResponse::set_response_code(StatusCode code)
{
	this->statuscode = status_codes[code][0];
	this->statusmsg = status_codes[code][1];
}


const char *HttpResponse::status_codes[61][2] = {
	{ "100", "Continue" },
	{ "101", "Switching Protocols" },
	{ "102", "Processing" },
	{ "103", "Early Hints" },
	{ "200", "OK" },
	{ "201", "Created" },
	{ "202", "Accepted" },
	{ "203", "Non-Authoritative Information" },
	{ "204", "No Content" },
	{ "205", "Partial Content" },
	{ "206", "Partial Content" },
	{ "207", "Multi-Status" },
	{ "208", "Already Reported" },
	{ "226", "IM Used" },
	{ "300", "Multiple Choices" },
	{ "301", "Moved Permanently" },
	{ "302", "Found" },
	{ "303", "See Other" },
	{ "304", "Not Modified" },
	{ "307", "Temporary Redirect" },
	{ "308", "Permanent Redirect" },
	{ "400", "Bad Request" },
	{ "401", "Unauthorized" },
	{ "402", "Payment Required" },
	{ "403", "Forbidden" },
	{ "404", "Not Found" },
	{ "405", "Method Not Allowed" },
	{ "406", "Not Acceptable" },
	{ "407", "Proxy Authentication Required" },
	{ "408", "Request Timeout" },
	{ "409", "Conflict" },
	{ "410", "Gone" },
	{ "411", "Length Required" },
	{ "412", "Precondition Failed" },
	{ "413", "Content Too Large" },
	{ "414", "URI Too Long" },
	{ "415", "Unsupported Media Type" },
	{ "416", "Range Not Satisfiable" },
	{ "417", "Expectation Failed" },
	{ "418", "I'm a teapot" },
	{ "421", "Misdirected Request" },
	{ "422", "Unprocessable Content" },
	{ "423", "Locked" },
	{ "424", "Failed Dependency" },
	{ "425", "Too Early" },
	{ "426", "Upgrade Required" },
	{ "428", "Precondition Required" },
	{ "429", "Too Many Requests" },
	{ "431", "Request Header Fields Too Large" },
	{ "451", "Unavailable For Legal Reasons" },
	{ "500", "Internal Server Error" },
	{ "501", "Not Implemented" },
	{ "502", "Bad Gateway" },
	{ "503", "Service Unavailable" },
	{ "504", "Gateway Timeout" },
	{ "505", "HTTP Version Not Supported" },
	{ "506", "Variant ALso Negotiates" },
	{ "507", "Insufficient Storage" },
	{ "508", "Loop Detected" },
	{ "510", "Not Extended" },
	{ "511", "Network Authentication Required" },
};

