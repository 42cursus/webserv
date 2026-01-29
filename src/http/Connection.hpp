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

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "State.hpp"
#include "HttpTransaction.hpp"
#include "Router.hpp"
#include "CgiHandler.hpp"
#include "StaticFileHandler.hpp"

#ifndef CRLF
#define CRLF "\r\n"
#endif

class TCPServer;

class Connection {
public:
    enum Result {
        OK = 0,
        WANT_WRITE = 1,
        CLOSED = 2,
        ERROR = 3
    };

    enum e_status {
        REQ_HEADERS = 0,
        REQ_BODY,
        REQ_RESPONSE_READY,
        REQ_MAX
    };

    Connection();
    Connection(const Connection &other);
    ~Connection();

    void        setSrv(TCPServer* srv);
    void        setFd(int fd);
    int         getFd() const;

    Result      onReadable();
    Result      onWritable();

    void        closeSocketFd();
    e_status    getStatus() const;
    void        reset();
    void        clearRequest();

private:
    Connection& operator=(const Connection&);


    Result _recvFromClient();
    Result _sendToClient();

    // transport
    int         _fd;
    TCPServer*  _srv;

    // HTTP state
    e_status    _status;
    std::string _rawRequest;

    HttpRequest*  _req;
    HttpResponse* _res;

    char        _req_buffer[4096 + 1];

    size_t      extract_body(size_t nread, size_t old_size, size_t clcr_pos) const;
    void        parse_range(HttpResponse& res) const;
    void        handle_error_response(HttpResponse* res) const;
    HttpResponse* prepareResponse() const;
};

#endif //CONNECTION_HPP
