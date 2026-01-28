/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/28 00:07:28 by abelov            #+#    #+#             */
/*   Updated: 2026/01/28 00:07:29 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "State.hpp"

struct Location : public IBlock {
    std::string _root;// where it's getting redirected;
    std::string _path;// the redirect
    std::vector<std::string> _index;
    std::vector<std::string> _methods;
    size_t _max_body_size;
    bool _autoindex;
    std::vector<CGI> _cgi;
    Location();
    Location &operator=(const Location &copy);
};

#endif//LOCATION_HPP
