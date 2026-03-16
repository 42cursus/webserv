/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:27:07 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 16:27:07 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTDISPATCHER_HPP
#define REQUESTDISPATCHER_HPP

#include "HttpRequest.hpp"

class Connection;
class HttpResponse;

class RequestDispatcher {
public:
	static HttpResponse *dispatch(Connection &conn, HttpRequest &req);

private:
	static void prepareResponsePut(HttpRequest &req, HttpResponse &res);
	static void prepareResponsePost(HttpRequest &req, HttpResponse &res);
	static void prepareResponseDelete(Connection &conn, HttpRequest &req, HttpResponse &res);
};

#endif//REQUESTDISPATCHER_HPP
