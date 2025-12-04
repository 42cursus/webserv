/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoindexHandler.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:44:31 by abelov            #+#    #+#             */
/*   Updated: 2025/12/04 16:44:31 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AUTOINDEXHANDLER_HPP
#define AUTOINDEXHANDLER_HPP


class AutoindexHandler : public IHandler {
    bool handle(HttpRequest &req, HttpResponse &res);
};


#endif //AUTOINDEXHANDLER_HPP
