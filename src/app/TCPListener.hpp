/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPListener.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 15:57:09 by abelov            #+#    #+#             */
/*   Updated: 2025/12/04 15:57:09 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef TCPLISTENER_HPP
#define TCPLISTENER_HPP

#include <string>
#include <vector>
#include <stdint-gcc.h>

class TCPListener {
    int                 fd;
    std::string         ip;
    uint16_t            port;

    std::vector<class VirtualHost *> vhosts;
};


#endif //TCPLISTENER_HPP
