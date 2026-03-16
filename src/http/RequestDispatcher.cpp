/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestDispatcher.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:27:03 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 16:27:03 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "RequestDispatcher.hpp"
#include "CgiHandler.hpp"
#include "Connection.hpp"
#include "Prefix_suffix.hpp"
#include "StaticFileHandler.hpp"
#include "TCPServer.hpp"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

HttpResponse *RequestDispatcher::dispatch(Connection &conn, HttpRequest &req)
{
	HttpResponse *res = new HttpResponse();

	res->headers["Server"]	= "Webserv/0.69";
	TrieNode *redirect_trie = conn.getSrv().getCfg().http.server.redirect_trie;
	Redirect *redirect		= prefix_trie_search<Redirect>(redirect_trie, req.path);

	if (redirect != NULL && req.path == redirect->_path) {
		conn.handleRedirectResponse(res, redirect);
		return res;
	}

	TrieNode *loc_trie = conn.getSrv().getCfg().http.server.loc_trie;
	Location *location = prefix_trie_search<Location>(loc_trie, req.path);
	res->location = location;
	if (!res->location) {
		res->set_response_code(SC_404);
		conn.handleErrorResponse(res);
		return res;
	}

	res->filename = req.path.substr(res->location->_path.length(), req.path.length());
	res->set_response_code(SC_200);

	if (!req.is_method_permitted(res->location)) {
		res->set_response_code(SC_405);
		conn.handleErrorResponse(res);
		return res;
	}

	CGI *cgi = suffix_trie_search(res->location->cgi_trie, req.path);

	try {
		if (cgi != NULL) {
			if (conn.getParent() == NULL)
				throw TCPServer::GenericException();
			CgiHandler cgi_handler(req, *res->location, cgi->_script, *res);
			cgi_handler.wrkr = conn.getParent();

			StatusCode code = SC_200;
			if (cgi->_script != "php")
				code = cgi_handler.handlePHP(req, *res);
			else
				code = cgi_handler.handle(req, *res);

			res->chunked = true;
			res->set_response_code(code);
			conn.setStatus(Connection::HANDLING_CGI);
			return res;
		}

		switch (req.get_method()) {
			case (HttpRequest::GET): {
				StaticFileHandler handler(*res->location);
				StatusCode code = handler.handle(req, *res);
				res->set_response_code(code);
				return res;
			}
			case (HttpRequest::PUT):
				prepareResponsePut(req, *res);
				break;
			case (HttpRequest::POST):
				prepareResponsePost(req, *res);
				break;
			case (HttpRequest::DELETE):
				prepareResponseDelete(conn, req, *res);
				break;
			default:
				res->set_response_code(SC_501);
				conn.handleErrorResponse(res);
				return res;
		}
	} catch (HttpResponse::Exception404 &) {
		res->set_response_code(SC_404);
		conn.handleErrorResponse(res);
		return res;
	} catch (HttpResponse::Exception403 &) {
		res->set_response_code(SC_403);
		conn.handleErrorResponse(res);
		return res;
	} catch (HttpResponse::Exception30x &) {
		conn.handleDirectoryRedirect(res);
		return res;
	} catch (std::exception &) {
		res->set_response_code(SC_500);
		conn.handleErrorResponse(res);
		return res;
	}

	return res;
}

void RequestDispatcher::prepareResponsePut(HttpRequest &req, HttpResponse &res)
{
	std::string rel_path = req.path.substr(res.location->_path.length(), req.path.length());
	if (rel_path.empty())
		rel_path = "default";
	std::string path = res.location->_root + rel_path;

	if (access(path.c_str(), F_OK) == 0)
		res.set_response_code(SC_204);
	else
		res.set_response_code(SC_201);

	res.headers["content-length"] = "0";
	int fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
	write(fd, &req.body[0], req.body.size());
	close(fd);
}

void RequestDispatcher::prepareResponsePost(HttpRequest &req, HttpResponse &res)
{
	(void) req;
	(void) res;
}

void RequestDispatcher::prepareResponseDelete(Connection &conn, HttpRequest &req, HttpResponse &res)
{
	std::string path = res.location->_root + res.filename;

	if (access(path.c_str(), F_OK) != 0) {
		res.set_response_code(SC_404);
		conn.handleErrorResponse(&res);
		return;
	}

	if (access(path.c_str(), W_OK) != 0) {
		res.set_response_code(SC_403);
		conn.handleErrorResponse(&res);
		return;
	}

	int retval = std::remove(path.c_str());
	if (retval == 0) {
		res.set_response_code(SC_204);
		res.headers["content-length"] = "0";
	} else {
		; // handle_error
	}
	(void) req;
}
