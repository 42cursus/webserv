/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 01:32:53 by abelov            #+#    #+#             */
/*   Updated: 2026/02/03 00:07:41 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpTransaction.hpp"
#include "Router.hpp"
#include "State.hpp"
#include "StaticFileHandler.hpp"

#include <deque>

#define REQUEST_BUF_SIZE 4096
#define RESPONSE_MSG_SIZE 4096
#ifndef CRLF
#define CRLF "\r\n"
#endif

class TCPServer;
class ConnWorker;

class Connection {
public:
	enum e_result {
		OK		   = 0,
		WANT_WRITE = 1,
		CLOSED	   = 2,
		ERROR	   = 3
	};

	enum e_status {
		READING_HEADERS = 0,
		READING_BODY,
		HANDLING_CGI,
		READY_TO_WRITE
	};

	Connection();
	explicit Connection(int fd, TCPServer *srv);
	Connection(const Connection &other);
	~Connection();

	void		setSrv(TCPServer *srv);
	TCPServer const&	getSrv(void) const;
	void		setFd(int fd);
	int			getFd() const;
	ConnWorker *getParent() const;
	void		setParent(ConnWorker *parent);
	e_status	getStatus() const;

	e_result onReadable();
	e_result onWritable();


	bool hasPendingResponses() const;
	void handleErrorResponse(HttpResponse *res) const;
	void handleRedirectResponse(HttpResponse *res, Redirect *redir) const;
	void handleDirectoryRedirect(HttpResponse *res) const;
	void setIpStr(std::string ip);
	std::string const& getIpStr(void) const;
	void closeSocketFd();
	void reset();
	void clearRequest();

private:
	Connection &operator=(const Connection &);

	e_result _recvFromClient();
	e_result _sendToClient();
	e_result _processInput();

	bool _tryExtractOneRequest();
	void _consumeInputBytes(size_t nbytes);
	void _resetCurrentRequest();

	bool _shouldKeepAlive(const HttpRequest &req) const;

	std::string	_ip;

	// transport
	int		   _fd;
	TCPServer *_srv;

	// HTTP state
	e_status _status;

	HttpRequest *_req;
	//    HttpResponse*   _res;

	// input buffering
	std::string _in;
	size_t		_in_off;

	bool _peerClosedInput;// read() returned 0 at least once

	struct PendingResponse {
		HttpResponse *res;
		bool		  closeAfter;// close connection after this response is fully sent
	};

	// output queue
	std::deque<PendingResponse> _pendingResponses;
	ConnWorker				   *_parent;
	char						_req_buffer[REQUEST_BUF_SIZE + 1];

	void _parseRange(HttpResponse &res) const;

	HttpResponse *_prepareResponse();
	void		  _prepareResponse_get(HttpResponse *res) const;
	void		  _prepareResponse_put(HttpResponse *res) const;
	void		  _prepareResponse_delete(HttpResponse *res) const;
	void		  _prepareResponse_post(HttpResponse *res) const;
};

#endif//CONNECTION_HPP
