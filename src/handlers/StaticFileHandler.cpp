/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 14:42:58 by abelov            #+#    #+#             */
/*   Updated: 2026/01/20 14:42:58 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "StaticFileHandler.hpp"
#include "HttpResponse.hpp"
#include "webserv.hpp"

/*
** -------------------------------- STATIC VARS -------------------------------
*/

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

StaticFileHandler::StaticFileHandler(const Location &loc) : _loc(loc) {}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/


/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

void StaticFileHandler::parse_range(HttpRequest &req, HttpResponse &res)
{
	std::string	rangestr = req.headers["range"];
	size_t		i = rangestr.find('=');
	char		*endptr;
	size_t		start = std::strtol(&rangestr.c_str()[i + 1], &endptr, 10);

	if (*endptr != '-')
		return ;
	endptr++;
	size_t		end = std::strtol(endptr, &endptr, 10);
	if (end == 0)
		end = res.body.length() - 1;
	res.headers["content-range"] = "bytes " + ::itoa(start) + "-" + ::itoa(end) + "/" + ::itoa(res.body.length());
	if (end < res.body.length() - 1)
		res.body.erase(end + 1);
	res.body.erase(0, start);
}

StatusCode StaticFileHandler::handle(HttpRequest &req, HttpResponse &res)
{
	StatusCode status = req.getHtmlResponse(res, const_cast<Location *>(&_loc));

	if (req.headers["range"].empty())
		res.headers["accept-ranges"] = "bytes";
	else if (status == SC_200 && req.headers["range"].find("bytes") == 0)
	{
		parse_range(req, res);
		status = SC_206;
	}
	if (res.headers.find("content-type") == res.headers.end() || res.headers["content-type"].empty())
		res.headers["content-type"] = "application/octet-stream";

	res.headers["content-length"] = ::itoa(static_cast<int>(res.body.size()));
	return status;
}

/*
** --------------------------------- METHODS ----------------------------------
*/

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/

