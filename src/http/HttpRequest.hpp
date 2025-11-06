/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:40:39 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 21:40:40 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP


#include <string>
#include <map>
#include "webserv.hpp"

class HttpRequest {

private:
	
	size_t	_parseStartLine(const std::string &line);
	size_t	_parseHeader(const std::string &line);

public:
	HttpRequest();

	std::string method;
	std::string path;
	std::string protocol;
	std::map<std::string, std::string> headers; //FIXME: we are probably not allowed to use map ???

	explicit HttpRequest(const std::string &path);

	void parseRequest(const std::string& rawRequest);

	std::string readHtmlFile(const std::string &, const Config &conf);
	std::string getHtmlResponse(const Config &conf);
	std::string getMimeType(const std::string &path);

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};
};

#endif //HTTPREQUEST_HPP
