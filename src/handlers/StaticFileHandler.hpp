/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticFileHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 14:41:53 by abelov            #+#    #+#             */
/*   Updated: 2026/01/20 14:41:54 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATICFILEHANDLER_HPP
#define STATICFILEHANDLER_HPP

#include "HttpResponse.hpp"
#include "IHandler.hpp"
#include "Location.hpp"

class StaticFileHandler  : public IHandler {
public:
    explicit StaticFileHandler(const Location& loc);
	StatusCode handle(HttpRequest& req, HttpResponse& res);
private:
    const Location& _loc;
};


#endif //STATICFILEHANDLER_HPP
