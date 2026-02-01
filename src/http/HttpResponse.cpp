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

/*
** -------------------------------- STATIC VARS -------------------------------
*/
HttpResponse::StatusCodeInitializer HttpResponse::status_code_initializer;
const char *HttpResponse::_status_codes[HTTP_RESPONSE_STATUS_CODES][2] = {
    { "", "" }
};

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

HttpResponse::StatusCodeInitializer::StatusCodeInitializer() {

    // Informational
    _set_status(SC_100, "100", "Continue" );
    _set_status(SC_101, "101", "Switching Protocols" );
    _set_status(SC_102, "102", "Processing" );
    _set_status(SC_103, "103", "Early Hints" );

    // Success
    _set_status(SC_200, "200", "OK" );
    _set_status(SC_201, "201", "Created" );
    _set_status(SC_202, "202", "Accepted" );
    _set_status(SC_203, "203", "Non-Authoritative Information" );
    _set_status(SC_204, "204", "No Content" );
    _set_status(SC_205, "205", "Partial Content" );
    _set_status(SC_206, "206", "Partial Content" );
    _set_status(SC_207, "207", "Multi-Status" );
    _set_status(SC_208, "208", "Already Reported" );
    _set_status(SC_226, "226", "IM Used" );

    // Redirection
    _set_status(SC_300, "300", "Multiple Choices" );
    _set_status(SC_301, "301", "Moved Permanently" );
    _set_status(SC_302, "302", "Found" );
    _set_status(SC_303, "303", "See Other" );
    _set_status(SC_304, "304", "Not Modified" );
    _set_status(SC_307, "307", "Temporary Redirect" );
    _set_status(SC_308, "308", "Permanent Redirect" );

    // Client error
    _set_status(SC_400, "400", "Bad Request" );
    _set_status(SC_401, "401", "Unauthorized" );
    _set_status(SC_402, "402", "Payment Required" );
    _set_status(SC_403, "403", "Forbidden" );
    _set_status(SC_404, "404", "Not Found" );
    _set_status(SC_405, "405", "Method Not Allowed" );
    _set_status(SC_406, "406", "Not Acceptable" );
    _set_status(SC_407, "407", "Proxy Authentication Required" );
    _set_status(SC_408, "408", "Request Timeout" );
    _set_status(SC_409, "409", "Conflict" );
    _set_status(SC_410, "410", "Gone" );
    _set_status(SC_411, "411", "Length Required" );
    _set_status(SC_412, "412", "Precondition Failed" );
    _set_status(SC_413, "413", "Content Too Large" );
    _set_status(SC_414, "414", "URI Too Long" );
    _set_status(SC_415, "415", "Unsupported Media Type" );
    _set_status(SC_416, "416", "Range Not Satisfiable" );
    _set_status(SC_417, "417", "Expectation Failed" );
    _set_status(SC_418, "418", "I'm a teapot" );
    _set_status(SC_421, "421", "Misdirected Request" );
    _set_status(SC_422, "422", "Unprocessable Content" );
    _set_status(SC_423, "423", "Locked" );
    _set_status(SC_424, "424", "Failed Dependency" );
    _set_status(SC_425, "425", "Too Early" );
    _set_status(SC_426, "426", "Upgrade Required" );
    _set_status(SC_428, "428", "Precondition Required" );
    _set_status(SC_429, "429", "Too Many Requests" );
    _set_status(SC_431, "431", "Request Header Fields Too Large" );
    _set_status(SC_451, "451", "Unavailable For Legal Reasons" );

    // Server error
    _set_status(SC_500, "500", "Internal Server Error" );
    _set_status(SC_501, "501", "Not Implemented" );
    _set_status(SC_502, "502", "Bad Gateway" );
    _set_status(SC_503, "503", "Service Unavailable" );
    _set_status(SC_504, "504", "Gateway Timeout" );
    _set_status(SC_505, "505", "HTTP Version Not Supported" );
    _set_status(SC_506, "506", "Variant ALso Negotiates" );
    _set_status(SC_507, "507", "Insufficient Storage" );
    _set_status(SC_508, "508", "Loop Detected" );
    _set_status(SC_510, "510", "Not Extended" );
    _set_status(SC_511, "511", "Network Authentication Required" );
    _set_status(SC_511, "511", "Network Authentication Required" );

    // Special non-standard
    _set_status(SC_MAX, "599", "Fintan is fuming with anger" ); // FIXME: >:( - Fin
}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/


/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

void HttpResponse::StatusCodeInitializer::_set_status(int code, const char *num, const char *msg) {
    if (code < 0 || code >= 600)
        return;
    _status_codes[code][0] = num;
    _status_codes[code][1] = msg;
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/

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
	std::string directory = location->_path + this->filename;
	std::vector<std::string>	filenames = _get_directory_members(location->_root + this->filename);
	std::vector<std::string>::iterator	it = filenames.begin();
	struct stat	statbuf;

	this->body += "<html>\n<head><title>Index of " + directory + "</title></head>\n";
	this->body += "<body>\n<h1>Index of " + directory + "</h1><hr><pre>";
	
	for (; it != filenames.end(); it++)
	{
		std::string path = location->_root + this->filename + *it;
		std::string line;
		std::string size;

		stat(path.c_str(), &statbuf);
		if (S_ISDIR(statbuf.st_mode))
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
	this->statuscode = _status_codes[code][0];
	this->statusmsg = _status_codes[code][1];
}
