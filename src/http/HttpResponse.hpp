/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:40:47 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 21:40:48 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "State.hpp"
#include "webserv.hpp"
#include <map>
#include <string>

class HttpResponse {

private:
	static const int   HTTP_RESPONSE_STATUS_CODES = 600;
	static const char *_status_codes[HTTP_RESPONSE_STATUS_CODES][2];
	struct StatusCodeInitializer {
		StatusCodeInitializer();
		static void _set_status(int code, const char *num, const char *msg);
	};

	__attribute__((unused)) static StatusCodeInitializer status_code_initializer;

public:
	std::map<std::string, std::string> headers;

	std::string statuscode;
	std::string statusmsg;
	std::string body;
	std::string filename;
	std::string response;
	size_t		start;
	Location	*location;
	bool		chunked;
	size_t		chunk_start;
	bool		chunking_express;
	bool		body_complete;

	HttpResponse();
	~HttpResponse();

	void	   buildAutoindexBody(void);
	void	   buildDefaultErrorPage(void);
	void	   buildHttpResponse(void);
	StatusCode readHtmlFile(const std::string &filename);
	std::string	chunk_response(size_t chunk_size);


	class GenericException : public std::exception {
	public:
		const char *what() const throw();
	};

	class Exception404 : public std::exception {
	public:
		const char *what() const throw();
	};

	class Exception403 : public std::exception {
	public:
		const char *what() const throw();
	};

	class Exception30x : public std::exception {
	public:
		const char *what() const throw();
	};

	void set_response_code(StatusCode code);
};


#endif//HTTPRESPONSE_HPP
