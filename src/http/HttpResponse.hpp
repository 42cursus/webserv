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

#include <string>
#include <map>
#include "webserv.hpp"
#include "State.hpp"

class HttpResponse {

private:

	static const char	*status_codes[61][2];

public:
	std::string statuscode;
	std::string statusmsg;
	std::map<std::string, std::string> headers;
	std::string body;
	std::string filename;
	std::string	response;
	size_t		start;
	Location	*location;

	void		buildAutoindexBody(void);
	void		buildHttpResponse(void);
	StatusCode	readHtmlFile(const std::string &filename);
	HttpResponse();

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};

	class Exception404 : public  std::exception
	{
	public:
		const char* what() const throw();
	};

	
	void		set_response_code(StatusCode code);

};


#endif //HTTPRESPONSE_HPP
