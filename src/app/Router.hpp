/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:34:28 by abelov            #+#    #+#             */
/*   Updated: 2025/12/04 16:34:28 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef WEBSERV_ROUTER_HPP
#define WEBSERV_ROUTER_HPP


#include "HttpRequest.hpp"
#include "Location.hpp"
#include "State.hpp"

class VirtualHost;

class Listener;

class Router {
public:
    const VirtualHost *resolve_vhost(const Listener &listener,
                                     const HttpRequest &req) const;

    const Location *resolve_location(const VirtualHost &vhost,
                                     const HttpRequest &req) const;
};


#endif //WEBSERV_ROUTER_HPP
