/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   State.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 20:47:27 by margo             #+#    #+#             */
/*   Updated: 2025/08/08 16:35:31 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "State.hpp"
#include "Parser.hpp"

IBlock::~IBlock() {};

std::string    IBlock::getName() const
{
    return _name;
}

std::string IBlock::getCode() const
{
    return _code;
}

std::vector<IDirective> IBlock::getDirectives() const
{
    return _directives;
}

IDirective* IBlock::getParent() const
{
    return _parent;
}

void    IBlock::setParent(IDirective*   parentDirective)
{
    _parent = parentDirective;
}

void    IBlock::toggle(Parser* parser)
{
    //toggle into specific block based on keyword
}

IDirective::~IDirective() {};

std::string IDirective::getName() const
{
    return _name;
}

std::vector<Parameter>  IDirective::getParameters() const
{
    return _parameters;
}

std::vector<IDirective> IDirective::getDirectives() const
{
    return _directives;
}

IBlock* IDirective::getBlock() const
{
    return _block;
}

IDirective* IDirective::getParent() const
{
    return _parent;
}

void    IDirective::setParent(IDirective* parentDirective)
{
    _parent = parentDirective;
}

int IDirective::getLine() const
{
    return _line;
}

void    IDirective::setLine(int line)
{
    _line = line;
}

void    IDirective::toggle(Parser*  parser)
{
    //set directive according to keyword
}