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

StatusCode StaticFileHandler::handle(HttpRequest &req, HttpResponse &res)
{
	StatusCode status = req.getHtmlResponse(res, const_cast<Location *>(&_loc));

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

