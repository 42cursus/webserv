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
#include "HttpResponse.hpp"
#include "webserv.hpp"

std::string itoa(int value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

std::string HttpResponse::buildHttpResponse(std::string statuscode,
								std::string statusmsg, std::map<std::string,
								std::string> headers, std::string body,
								std::string mimetype)
{
	std::ostringstream buffer;

	// headers["content-type"] = mimetype;
	// headers["content-length"] = itoa(body.length());
	buffer << "HTTP/1.1 " << statuscode << " " << statusmsg << "\r\n";

	for (StringMap::iterator it = headers.begin(); it != headers.end(); ++it)
		buffer << it->first << ": " << it->second << "\r\n";
	buffer << "\r\n" << body;
	return buffer.str();
	(void)mimetype;
}
