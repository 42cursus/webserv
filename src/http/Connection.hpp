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

class Listener;
class VirtualHost;

class Connection {
private:
    static char _inBuffer[1024];
    static char _outBuffer[1024];
    int                  fd;
    Listener            *listener;   // which listener accepted this connection
    HttpTransaction     tx;
    const VirtualHost   *current_vhost;

    bool keep_alive;
    bool has_request;
    bool response_ready;

    void onReadable();
    void onWritable();

    void onRequestParsed(Router router, HttpRequest &req);

};


#endif //CONNECTION_HPP
