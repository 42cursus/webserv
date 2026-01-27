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
#include "HttpResponse.hpp"
#include "State.hpp"
#include "webserv.hpp"

#define MAX_BODY_SIZE 1000
#define DECIMAL_BASE 10

class HttpRequest {

private:
	
	size_t	_parseStartLine(const std::string &line);
	size_t	_parseHeader(const std::string &line);

public:

	HttpRequest();

	std::string			method;
	std::string			path;
	std::string			protocol;
	std::vector<char>	body;
	size_t				content_length;
	std::map<std::string, std::string> headers;

	explicit HttpRequest(const std::string &path);

	void parseRequest(const std::string& rawRequest);

	std::string getHtmlResponse(const Location *conf, HttpResponse& res);
	std::string getMimeType(const std::string &path);
	void		printBody() const;

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};
};

#endif //HTTPREQUEST_HPP
