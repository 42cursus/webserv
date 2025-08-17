/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   State.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 20:47:27 by margo             #+#    #+#             */
/*   Updated: 2025/08/12 21:35:05 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "State.hpp"
#include "Parser.hpp"

IBlock::IBlock(std::string name): _name(name) {};

IBlock::~IBlock() 
{
    delete _parent;

    for (std::vector<IDirective*>::iterator it = _directives.begin(); it != _directives.end(); ++it)
        delete *it;
    _directives.clear();
}

std::string    IBlock::getName() const
{
    return _name;
}

std::string IBlock::getCode() const
{
    return _code;
}

std::vector<IDirective*> IBlock::getDirectives() const
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

IDirective::IDirective(std::string name): _name(name) {};

IDirective::~IDirective()
{
    delete _block;
    delete _parent;
    
    for (std::vector<IDirective*>::iterator it = _directives.begin(); it != _directives.end(); ++it)
        delete *it;
    _directives.clear();
}

std::string IDirective::getName() const
{
    return _name;
}

std::vector<Parameter>  IDirective::getParameters() const
{
    return _parameters;
}

std::vector<IDirective*> IDirective::getDirectives() const
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

HTTP::HTTP(): IBlock("http")
{
    setParent(NULL);
}

HTTP::~HTTP() {};