/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 04:08:12 by abelov            #+#    #+#             */
/*   Updated: 2026/02/03 00:28:34 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include "ConnWorker.hpp"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <exception>
#include <stdexcept>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"
#include "Logging.hpp"
#include "Prefix_suffix.hpp"
#include "RequestParser.hpp"
#include "State.hpp"
#include "TCPServer.hpp"
#include "webserv.hpp"

#include "CgiHandler.hpp"
#include "StaticFileHandler.hpp"

/*
** -------------------------------- STATIC VARS -------------------------------
*/

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

Connection::Connection() :
	_fd(-1),
	_srv(NULL),
	_status(READING_HEADERS),
	_req(NULL),
	_inOffset(0),
	_peerClosedInput(false),
	_parent(NULL),
	_req_buffer()
{}

bool Connection::hasPendingResponses() const
{
	return !_pendingResponses.empty();
}

Connection::Connection(const Connection &other) :
	_fd(other._fd),
	_srv(other._srv),
	_status(other._status),
	_req(other._req),
	_inOffset(other._inOffset),
	_peerClosedInput(other._peerClosedInput),
	_pendingResponses(other._pendingResponses),
	_parent(other._parent)
{}

Connection::Connection(int fd, TCPServer *srv) :
	_fd(fd),
	_srv(srv),
	_status(READING_HEADERS),
	_req(NULL),
	_inOffset(0),
	_peerClosedInput(false),
	_req_buffer()
{
	int flags = fcntl(_fd, F_GETFL, 0);
	if (flags >= 0)
		fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/

Connection::~Connection()
{
	if (_fd != -1) {
		::close(_fd);
		_fd = -1;
	}
	reset();
}

/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

Connection::e_result Connection::_recvFromClient()
{
	while (true) {
		ssize_t nread = ::read(_fd, _req_buffer, REQUEST_BUF_SIZE);
		if (nread > 0) {
			_req_buffer[nread] = '\0';
			_inputBuffer.append(_req_buffer, static_cast<size_t>(nread));
			continue; // drain the kernel buffer
		}
		if (nread == 0) {
			// Peer closed its write-side (FIN).
			// We might still have a full request in _in.
			_peerClosedInput = true;
			return OK;
		}

		if (_status == READING_BODY) {
			size_t old_size = _req->body.size();
			_req->body.resize(old_size + nread);
			std::memcpy(_req->body.data() + old_size, _req_buffer, nread);
			//_req->printBody();
		}

		if (errno == EINTR)  // FIXME: CAN'T DO THAT!!!
			continue;
		if (errno == EAGAIN || errno == EWOULDBLOCK)  // FIXME: CAN'T DO THAT!!!
			return OK;

		return ERROR;
	}
}

void logServingFile(const std::string &path, const std::string &mimetype)
{
	std::cout << "Serving file: " << path << " with MIME type: " << mimetype << std::endl;
}

Connection::e_result Connection::_sendToClient()
{
	if (_pendingResponses.empty())
	{
		// std::cout << "Returning OK as queue empty" << std::endl;
		return OK;
	}

	PendingResponse &item = _pendingResponses.front();
	if (!item.res)
		return ERROR;

    HttpResponse* cur = item.res;
	
	// if (cur->chunked && cur->chunking_ready)
	// 	return _sendToClientChunked(item);

    std::string& response = cur->response;
    if (cur->start >= response.size())
	{
		if (cur->chunk_start < cur->body.size() || cur->body_complete)
		{
			cur->start = 0;
			cur->response = cur->chunk_response(32768);
			if (cur->body_complete)
			{
				cur->chunking_express = true;
				// cur->response += CHUNK_END;
			}
			log_chunk_response(*this, *cur);
			// return WANT_WRITE;
		}
		else if (!cur->chunked)
		{
			// std::cout << "Returning OK as start > response.size()" << std::endl;
        	return OK;
		}
		else
			return WANT_WRITE;
	}
    if (cur->start == 0 && cur->chunk_start == 0)
    // if (cur->start == 0)
		log_response(*this, *cur);

    size_t	remaining = response.length() - cur->start;
    // resize_socket_buffer(_conn_fd, msg_size);
    remaining = std::min(remaining, response.length() - cur->start);
    ssize_t bytesWritten = ::write(_fd, response.data() + cur->start, remaining);

    if (bytesWritten > 0)
    {
        cur->start += static_cast<size_t>(bytesWritten);
        if (cur->start >= response.size())
        {
			if (cur->chunked)
			{
				if (!cur->chunking_express)
				{
					if (cur->chunk_start >= cur->body.size())
					{
						if (cur->body_complete)
							cur->chunking_express = true;
						else
							return WANT_WRITE;
					}
					cur->start = 0;
					cur->response = cur->chunk_response(32768);
					log_chunk_response(*this, *cur);
					return WANT_WRITE;
				}
			}

            const bool close_after = item.closeAfter;

			delete cur;
			_pendingResponses.pop_front();

			// If the request said "Connection: close",
			// close *after* we sent its response.
			if (close_after)
				return CLOSED;

			if (!_pendingResponses.empty())
				return WANT_WRITE;

			_status = READING_HEADERS;

			// try to parse / enqueue immediately
			// so we don't wait for another EPOLLIN.
			e_result pr = _processInput();
			if (pr == WANT_WRITE)
				return WANT_WRITE;
			// std::cout << "Returning OK as response complete" << std::endl;
			return OK;
		}
		return WANT_WRITE;
	}

	if (bytesWritten == 0)
		return CLOSED;

	if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) // FIXME: CAN'T DO THAT!!!
		return WANT_WRITE;

	if (errno == EPIPE || errno == ECONNRESET) // FIXME: CAN'T DO THAT!!!
		return CLOSED;

	return ERROR;
}

// Connection::e_result Connection::_sendToClientChunked(PendingResponse &item)
// {
//     HttpResponse*	cur = item.res;
// 	const size_t	body_size = cur->body.size();
//
//     if (cur->start >= body_size)
//         return OK;
//
// }

void Connection::_consumeInputBytes(size_t nbytes)
{
	_inOffset += nbytes;

	size_t magic_nbr = 65536; // FIXME: magic-number...

	if (_inOffset > magic_nbr || (_inOffset > 0 && _inOffset * 2 > _inputBuffer.size())) {
		_inputBuffer.erase(0, _inOffset);
		_inOffset = 0;
	}
}

void Connection::_resetCurrentRequest()
{
	delete _req;
	_req	= NULL;
	_status = READING_HEADERS;
}

/**
 * - HTTP/1.1 => keep-alive by default unless "connection: close"
 * - HTTP/1.0 => close by default unless "connection: keep-alive"
 * @param req
 * @return
 */
bool Connection::_shouldKeepAlive(const HttpRequest &req) const
{
	std::string proto = req.protocol;
	std::string conn;

	if (req.headers.count("connection"))
		conn = req.headers.find("connection")->second;

	for (size_t i = 0; i < conn.size(); i++) // normalize to lowercase
		conn[i] = static_cast<char>(std::tolower(conn[i]));

	if (proto.find("HTTP/1.1") == 0)
		return conn != "close";
	return conn == "keep-alive";
}

/**
 * if we have headers (and body if needed),
 * consume bytes and enqueue a response”.
 * @return
 */
bool Connection::_tryExtractOneRequest()
{
	RequestParser::Result extract = RequestParser::tryExtract(_inputBuffer, _inOffset);
	if (extract.status == RequestParser::NEED_MORE_DATA)
		return false;

	if (extract.status == RequestParser::BAD_REQUEST) {
		_inputBuffer.erase();
		delete _req;
		_req = NULL;

		std::runtime_error parse_err(extract.error_message.empty() ? "Bad request" : extract.error_message);
		log_request_error(*this, parse_err);

		HttpResponse *res = new HttpResponse();
		res->set_response_code(SC_400);
		res->headers["Server"] = "Webserv/0.69";
		handleErrorResponse(res);
		res->buildHttpResponse();

		const PendingResponse &presp = (PendingResponse){
			.res = res,
			.closeAfter = true
		};
		_pendingResponses.push_back(presp);

		_status = READY_TO_WRITE;
		return false;
	}

	_consumeInputBytes(extract.consumed_bytes);

	delete _req;
	_req = extract.request;

	log_request(*this, *_req);
	// std::cout << this->_req_buffer;
    HttpResponse* res = _prepareResponse();
	if (_status == HANDLING_CGI)
	{
		res->buildHttpResponse();
		PendingResponse presp;
		presp.res		 = res;
		presp.closeAfter = !_shouldKeepAlive(*_req);

		_pendingResponses.push_back(presp);
		return true;
	}


	res->buildHttpResponse();
	// _req->printBody();

	PendingResponse presp;
	presp.res		 = res;
	presp.closeAfter = !_shouldKeepAlive(*_req);

	_pendingResponses.push_back(presp);

	_status = READY_TO_WRITE;
	return true;
}

/**
 * Repeatedly try to parse as many complete requests as are already
 * buffered and append responses to the queue.
 * @see HTTP/1.1 pipelining.
 * @return
 */
Connection::e_result Connection::_processInput()
{
	// If we're currently writing, we still can parse and enqueue more
	// pipelined requests, but we should not drop back to EPOLLIN-only
	// if there's data to send.
	bool queued_any = false;

	while (true) {
		const size_t before = _pendingResponses.size();
		if (!_tryExtractOneRequest())
			break;
		if (_pendingResponses.size() > before)
			queued_any = true;
	}

	if (!_pendingResponses.empty()) // switching to EPOLLOUT
		return WANT_WRITE;

	return queued_any ? WANT_WRITE : OK;
}

Connection::e_result Connection::onReadable()
{
	return _handleReadable();
}

Connection::e_result Connection::onWritable()
{
	return _handleWritable();
}

void Connection::enqueueResponse(PendingResponse &req)
{
	_pendingResponses.push_back(req);
}

HttpResponse *Connection::getCurrentResponse() const
{
	if (_pendingResponses.empty())
		return NULL;

	return _pendingResponses.front().res;
}

bool Connection::_wantWrite() const
{
	return !_pendingResponses.empty() && _status == READY_TO_WRITE;
}

Connection::e_result Connection::_handleReadable()
{
	e_result rr = _recvFromClient();
	if (rr != OK)
		return rr;

	// parse/enqueue as much as possible
	e_result pr = _processInput();
	if (pr == WANT_WRITE)
		return WANT_WRITE;

	// If the peer already closed input, and we produced nothing to write, close now.
	if (_peerClosedInput && _pendingResponses.empty())
		return CLOSED;

	return _wantWrite() ? WANT_WRITE : OK;
}

Connection::e_result Connection::_handleWritable()
{
	// if (_status != READY_TO_WRITE)
	// {
	// 	std::cout << "Returning OK in handleWritable" << std::endl;
	// 	return OK;
	// }
	// return _sendToClient();
	return (_status != READY_TO_WRITE) ? OK : _sendToClient();
}

HttpResponse *Connection::_prepareResponse()
{
	HttpResponse *res = new HttpResponse();

	res->headers["Server"]	= "Webserv/0.69";
	TrieNode *redirect_trie = _srv->getCfg().http.server.redirect_trie;
	Redirect *redirect		= prefix_trie_search<Redirect>(redirect_trie, _req->path);

	if (redirect != NULL && _req->path == redirect->_path) {
		handleRedirectResponse(res, redirect);
		return res;
	}

	TrieNode *loc_trie = _srv->getCfg().http.server.loc_trie;
	Location *location = prefix_trie_search<Location>(loc_trie, _req->path);
	res->location	   = location;
	if (!res->location) {
		res->set_response_code(SC_404);
		handleErrorResponse(res);
		return res;
	}

	res->filename = _req->path.substr(res->location->_path.length(), _req->path.length());
	std::string mimetype;

	res->set_response_code(SC_200);

	if (!_req->is_method_permitted(res->location)) {
		res->set_response_code(SC_405);
		handleErrorResponse(res);
		return res;
	}

	CGI *cgi = suffix_trie_search(res->location->cgi_trie, _req->path);

	try {
		if (cgi != NULL) {
			if (this->_parent == NULL)
				throw new TCPServer::GenericException();
			CgiHandler cgi_handler(*_req, *res->location, cgi->_script, *res);
			cgi_handler.wrkr		= this->_parent;

			StatusCode code = SC_200;
			if (cgi->_script != "php")  // FIXME: what the heck???
				code = cgi_handler.handlePHP(*_req, *res);
			else
				code = cgi_handler.handle(*_req, *res);

			res->chunked = true;
			res->set_response_code(code);
			// if (res->headers.find("content-length") == res->headers.end()) {
			// 	// res->headers["content-length"] = ::size_to_ascii(res->body.size());
			// 	res->headers["content-length"] = ::size_to_ascii(19);
			// }
			this->_status = HANDLING_CGI;
			return res;
		}

		switch (_req->get_method()) {
			case (HttpRequest::GET): {
				// res->chunked = true;
				StaticFileHandler handler(*res->location);
				StatusCode		  code = handler.handle(*_req, *res);
				res->set_response_code(code);
				return res;
			}
			case (HttpRequest::PUT):
				_prepareResponse_put(res);
				break;
			case (HttpRequest::POST):
				_prepareResponse_post(res);
				break;
			case (HttpRequest::DELETE):
				_prepareResponse_delete(res);
				break;
			default:
				res->set_response_code(SC_501);
				handleErrorResponse(res);
				return res;
		}
	} catch (HttpResponse::Exception404 &) {
		res->set_response_code(SC_404);
		handleErrorResponse(res);
		return res;
	} catch (HttpResponse::Exception403 &) {
		res->set_response_code(SC_403);
		handleErrorResponse(res);
		return res;
	} catch (HttpResponse::Exception30x &) {
		handleDirectoryRedirect(res);
		return res;
	} catch (std::exception &) {
		res->set_response_code(SC_500);
		handleErrorResponse(res);
		return res;
	}
	return res;
}

void Connection::_prepareResponse_get(HttpResponse *res) const
{
	res->body = _req->getHtmlResponse(*res, res->location);

	if (_req->headers["range"].empty())
		res->headers["accept-ranges"] = "bytes";
	else if (_req->headers["range"].find("bytes") == 0) {
		_parseRange(*res);
		res->set_response_code(SC_206);
	}
	res->headers["content-length"] = ::itoa(res->body.length());
}

void Connection::_prepareResponse_put(HttpResponse *res) const
{
	std::string rel_path = _req->path.substr(res->location->_path.length(), _req->path.length());
	if (rel_path.empty())
		rel_path = "default";
	std::string path = res->location->_root + rel_path;

	if (access(path.c_str(), F_OK) == 0)
		res->set_response_code(SC_204);
	else
		res->set_response_code(SC_201);

	res->headers["content-length"] = "0";
	int fd = open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IROTH | S_IRGRP);
	write(fd, &_req->body[0], _req->body.size());
	close(fd);
}

void Connection::_prepareResponse_post(HttpResponse *res) const
{
	(void) res;
}

void Connection::_prepareResponse_delete(HttpResponse *res) const
{
	std::string path = res->location->_root + res->filename;

	if (access(path.c_str(), F_OK) != 0) {
		res->set_response_code(SC_404);
		handleErrorResponse(res);
		return;
	}

	if (access(path.c_str(), W_OK) != 0) {
		res->set_response_code(SC_403);
		handleErrorResponse(res);
		return;
	}

	int retval = std::remove(path.c_str());
	if (retval == 0) {
		res->set_response_code(SC_204);
		res->headers["content-length"] = "0";
	} else {
		; // handle_error
	}
}

void Connection::handleErrorResponse(HttpResponse *res) const
{
	std::map<std::string, std::string>::const_iterator path_it;

	path_it = _srv->getCfg().http.server.error_pages.find(res->statuscode);
	while (path_it != _srv->getCfg().http.server.error_pages.end()) {

		std::string	path = path_it->second;
		TrieNode *loc_trie = _srv->getCfg().http.server.loc_trie;
		Location *location = prefix_trie_search<Location>(loc_trie, path_it->second);

		if (location == NULL)
			break;

		path = apply_location(path, location);
		// std::cout << path << std::endl;
		res->headers["content-type"] = _req->getMimeType(path);
		try {
			res->readHtmlFile(path);
		} catch (HttpResponse::Exception404 &ex) {
			res->set_response_code(SC_404);
			(void) ex; // FIXME: do actually smth with `ex`
			break;
		}
		res->headers["content-length"] = ::itoa(res->body.length());
		return;
	}
	res->headers["content-type"] = "text/html";
	res->buildDefaultErrorPage();
	res->headers["content-length"] = ::itoa(res->body.length());
}

void Connection::handleRedirectResponse(HttpResponse *res, Redirect *redir) const
{
	res->set_response_code(redir->_code);
	res->headers["Location"] = redir->_redirect;
	handleErrorResponse(res);
}

void Connection::handleDirectoryRedirect(HttpResponse *res) const
{
	std::string path		 = res->location->_path + res->filename + '/';
	res->headers["Location"] = path;
	handleErrorResponse(res);
}

void Connection::_parseRange(HttpResponse &res) const
{
	std::string rangestr = _req->headers["range"];
	size_t		i		 = rangestr.find('=');
	char	   *endptr;
	size_t		start = std::strtol(&rangestr.c_str()[i + 1], &endptr, 0);

	if (*endptr != '-')
		return;
	endptr++;
	size_t end = std::strtol(endptr, &endptr, 0);
	if (end == 0)
		end = res.body.length() - 1;

	res.headers["content-range"] =
		"bytes " + ::itoa(start) + "-" + ::itoa(end) + "/" + ::itoa(res.body.length());
	if (end < res.body.length() - 1)
		res.body.erase(end + 1);
	res.body.erase(0, start);
}

void Connection::clearRequest()
{
	_inputBuffer.clear();
	_inOffset = 0;
	_resetCurrentRequest();
}

void Connection::closeSocketFd()
{
	int fd = _fd;
	if (fd != -1)
		::close(fd);
	_fd = -1;
}

void Connection::reset()
{
	// drain queued responses
	while (!_pendingResponses.empty()) {
		delete _pendingResponses.front().res;
		_pendingResponses.pop_front();
	}

	delete _req;
	_req = NULL;

	_inputBuffer.clear();
	_inOffset = 0;

	_peerClosedInput = false;
	_status			 = READING_HEADERS;
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

void Connection::setSrv(TCPServer *srv)
{
	_srv = srv;
}

TCPServer const &Connection::getSrv() const
{
	return *_srv;
}

void Connection::setFd(int fd)
{
	_fd		  = fd;
	int flags = fcntl(_fd, F_GETFL, 0);
	if (flags >= 0)
		fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
}

int Connection::getFd() const
{
	return _fd;
}

Connection::e_status Connection::getStatus() const
{
	return _status;
}

Connection::e_status Connection::setStatus(e_status status)
{
	_status = status;
	return _status;
}

ConnWorker *Connection::getParent() const
{
	return _parent;
}
void Connection::setParent(ConnWorker *parent)
{
	_parent = parent;
}

void	Connection::setIpStr(std::string ip)
{
	_ip = ip;
}

std::string const&	Connection::getIpStr(void) const
{
	return _ip;
}

/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/
