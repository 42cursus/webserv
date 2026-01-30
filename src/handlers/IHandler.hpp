/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHandler.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:44:00 by abelov            #+#    #+#             */
/*   Updated: 2025/12/04 16:44:00 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IHANDLER_HPP
#define IHANDLER_HPP

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "webserv.hpp"

class IHandler {
public:
    virtual ~IHandler() {}
    virtual StatusCode handle(HttpRequest& req, HttpResponse& res) = 0;
};

#endif //IHANDLER_HPP
