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

void Connection::onRequestParsed() {
    current_vhost = router.resolve_vhost(*listener, current_request);
    if (!current_vhost) {
        // 400 or 421 or 404, depending on how strict you want to be
        build_error_response(400);
        return;
    }

    current_location = router.resolve_location(*current_vhost, current_request);
    if (!current_location) {
        // fall back to some default location, likely "/"
    }

    if (current_location->is_cgi) {
        cgi_handler.handle(*this, *current_vhost, *current_location);
    } else {
        static_handler.handle(*this, *current_vhost, *current_location);
    }
}