/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 02:32:33 by abelov            #+#    #+#             */
/*   Updated: 2025/11/26 02:32:33 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_LOCATIONCONFIG_HPP
#define WEBSERV_LOCATIONCONFIG_HPP

#include "webserv.hpp"

class LocationConfig
{
public:
    LocationConfig(Config::Http::Server::Location &loc);
private:
    Config::Http::Server::Location::Conf _conf;
};


#endif //WEBSERV_LOCATIONCONFIG_HPP
