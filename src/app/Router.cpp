/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:35:04 by abelov            #+#    #+#             */
/*   Updated: 2025/12/04 16:35:04 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

const VirtualHost *Router::resolve_vhost(const Listener &listener, const HttpRequest &req) const {
    return NULL;
}

const Location *Router::resolve_location(const VirtualHost &vhost, const HttpRequest &req) const {
    return NULL;
}
