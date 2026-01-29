/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnWorker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 20:31:02 by abelov            #+#    #+#             */
/*   Updated: 2025/08/23 20:29:23 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef WORKER_HPP
#define WORKER_HPP

#include "Connection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "TCPServer.hpp"
#include <netinet/in.h>

#define REQUEST_BUF_SIZE 4096
#define RESPONSE_MSG_SIZE 4096
#define CRLF "\r\n"

class ConnWorker {
public:
    typedef Connection::e_status e_status;

    explicit ConnWorker();
    ConnWorker(const ConnWorker &other);
    ~ConnWorker();

    void setSrv(TCPServer *srv);
    void setConnFd(int connFd);

    int getConnFd() const;

    int onReadable();
    int onWritable();

    void reset();
    void clearRequest();

    e_status getStatus() const;

    void closeSocketFd();

    class GenericException : public std::exception {
    public:
        const char *what() const throw();
    };

    ConnWorker& operator=(const ConnWorker&);
    const Connection &getConn() const;

private:
    Connection _conn;
};
#endif //WORKER_HPP
