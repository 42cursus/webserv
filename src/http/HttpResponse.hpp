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

	void buildHttpResponse(void);
	std::string	readHtmlFile(const std::string &filename);
	HttpResponse();

	class GenericException : public  std::exception
	{
	public:
		const char* what() const throw();
	};

	enum e_statuscodes
	{
		SC_100,
		SC_101,
		SC_102,
		SC_103,
		SC_200,
		SC_201,
		SC_202,
		SC_203,
		SC_204,
		SC_205,
		SC_206,
		SC_207,
		SC_208,
		SC_226,
		SC_300,
		SC_301,
		SC_302,
		SC_303,
		SC_304,
		SC_307,
		SC_308,
		SC_400,
		SC_401,
		SC_402,
		SC_403,
		SC_404,
		SC_405,
		SC_406,
		SC_407,
		SC_408,
		SC_409,
		SC_410,
		SC_411,
		SC_412,
		SC_413,
		SC_414,
		SC_415,
		SC_416,
		SC_417,
		SC_418,
		SC_421,
		SC_422,
		SC_423,
		SC_424,
		SC_425,
		SC_426,
		SC_428,
		SC_429,
		SC_431,
		SC_451,
		SC_500,
		SC_501,
		SC_502,
		SC_503,
		SC_504,
		SC_505,
		SC_506,
		SC_507,
		SC_508,
		SC_510,
		SC_511,
	};
	void		set_response_code(e_statuscodes code);

};


#endif //HTTPRESPONSE_HPP
