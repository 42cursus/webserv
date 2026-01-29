/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 01:32:53 by abelov            #+#    #+#             */
/*   Updated: 2025/12/04 01:32:54 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "CgiHandler.hpp"
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

class Connection {
public:
    enum e_result {
        OK = 0,
        WANT_WRITE = 1,
        CLOSED = 2,
        ERROR = 3
    };

    enum e_status {
        READING_HEADERS = 0,
        READING_BODY,
        READY_TO_WRITE
    };

    Connection();
    explicit Connection(int fd, TCPServer* srv);
    Connection(const Connection &other);
    ~Connection();

    void        setSrv(TCPServer* srv);
    void        setFd(int fd);
    int         getFd() const;

    e_result    onReadable();
    e_result    onWritable();

    void        closeSocketFd();
    e_status    getStatus() const;
    void        reset();
    void        clearRequest();

private:
    Connection& operator=(const Connection&);

    e_result _recvFromClient();
    e_result _sendToClient();

    // incremental parsing helpers
    e_result _processInput();
    bool    _tryExtractOneRequest();
    void    _consume(size_t nbytes);
    void    _resetCurrentRequest();

    bool    _shouldKeepAlive(const HttpRequest& req) const;

    // transport
    int         _fd;
    TCPServer*  _srv;

    // HTTP state
    e_status    _status;
    std::string _raw;
    size_t      _header_end;

    HttpRequest*    _req;
    HttpResponse*   _res;
    size_t          _write_off;

    // input buffering
    std::string _in;
    size_t      _in_off;
    size_t      _expected_body;
    bool        _keep_alive_for_current;

    // output queue
    std::deque<HttpResponse*> _outq;

    char        _req_buffer[REQUEST_BUF_SIZE + 1];

    e_result    _readIntoBuffer();
    e_result    _tryParseRequest();
    void        _resetForNextRequest();

    void            _parseRange(HttpResponse& res) const;
    void            _handleErrorResponse(HttpResponse* res) const;
    HttpResponse*   _prepareResponse() const;
};

#endif //CONNECTION_HPP
