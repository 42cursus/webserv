/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 02:34:08 by abelov            #+#    #+#             */
/*   Updated: 2025/11/26 02:34:08 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig(Config::Http::Server::Location& loc)
  : _conf(loc.config)
{
}
