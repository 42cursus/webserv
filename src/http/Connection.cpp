/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 04:08:12 by abelov            #+#    #+#             */
/*   Updated: 2025/12/04 04:08:12 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include "Router.hpp"

void Connection::onRequestParsed(Router router, HttpRequest &current_request) {
    current_vhost = router.resolve_vhost(*listener, current_request);
    if (!current_vhost) {
        //build_error_response(400);  // 400 or 421 or 404
        return;
    }

    const Location *current_location;

    current_location = router.resolve_location(*current_vhost, current_request);
    if (!current_location) {
        // fall back to the default location
    }

    if (current_location->isCgi) {
        CgiHandler cgi_handler(current_location);

        cgi_handler.handle(current_request, );
    } else {
        StaticFileHandler static_handler(current_location.);
        static_handler.handle(current_request, *current_location);
    }
}
