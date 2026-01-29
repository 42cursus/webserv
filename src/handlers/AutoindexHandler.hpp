/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoindexHandler.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:44:31 by abelov            #+#    #+#             */
/*   Updated: 2026/01/20 14:42:06 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AUTOINDEXHANDLER_HPP
#define AUTOINDEXHANDLER_HPP

#include "IHandler.hpp"

class AutoindexHandler : public IHandler {
    explicit AutoindexHandler(const Location& loc);
    int handle(HttpRequest& req, HttpResponse& res);

private:
    const Location& _loc;
};

#endif //AUTOINDEXHANDLER_HPP
