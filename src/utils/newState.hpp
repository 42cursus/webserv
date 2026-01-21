/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newState.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 17:45:42 by margo             #+#    #+#             */
/*   Updated: 2026/01/21 18:13:36 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NEWSTATE_HPP
#define NEWSTATE_HPP

#include <iostream>

enum    e_block_type
{
    SERVER,
    LOCATION,
    CGI,
} ; 

struct  Directive
{
    std::string _key; // left side
    std::string _parameter; // right side;
    unsigned  int   _equal; // = index
} ;

class   IBlock
{
    private:
        unsigned int    _line;
        e_block_type    _type;
        IBlock* _parent_block; // if NULL we're in main server block
        
} ;

#endif