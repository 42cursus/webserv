/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   State.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 20:47:27 by margo             #+#    #+#             */
/*   Updated: 2025/08/25 23:14:59 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "State.hpp"
#include "Parser.hpp"

Start::Start() {};

Start::~Start() {};

void    Start::toggle(Parser *parser)
{
    if (parser->getCurrentToken().literal == "http")
    {
        parser->setCurrentState(new HTTP());
        parser->getCurrentState()->enter(parser);
    }
    // else if for events or other top level directives if needed
}

IBlock::IBlock(std::string name): _name(name) {};

bool IBlock::operator==(const IBlock& oth)
{
    return this->getName() == oth.getName();     
}

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

std::vector<IState*> IBlock::getDirectives() const
{
    return _directives;
}

void    IBlock::addDirective(IState* newDir)
{
    _directives.push_back(newDir);
}

IState* IBlock::getParent() const
{
    return _parent;
}

void    IBlock::setParent(IState*   parentDirective)
{
    _parent = parentDirective;
}

void    IBlock::toggle(Parser* parser)
{
    (void)parser;
    //toggle into specific block based on keyword
}

int IBlock::getLine() const
{
    return _line;
}

void    IBlock::setLine(int line)
{
    _line = line;
}

HTTP::HTTP(): IBlock("http") {};

HTTP::~HTTP() {};

void    HTTP::enter(Parser* parser)
{
    parser->setKey(getName());
    parser->setInBlock(true);
    parser->addNewBlock(this);
    setLine(parser->getCurrentToken().line);
    setParent(NULL);
}

void    HTTP::toggle(Parser* parser)
{
    if (parser->getCurrentToken().literal == "server")
    {
        parser->setCurrentState(new Server());
        exit(parser);
        parser->getCurrentState()->enter(parser);
    }
    
}

void    HTTP::exit(Parser* parser)
{
    addDirective(parser->getCurrentState());
}

Server::Server(): IBlock("server") {};

Server::~Server() {};

void    Server::enter(Parser *parser)
{
    parser->setKey(getName());
    parser->setInBlock(true);
    parser->addNewBlock(this);
    setLine(parser->getCurrentToken().line);
    setParent(parser->getBlock("http"));
}

void    Server::exit(Parser *parser)
{
    addDirective(parser->getCurrentState());
}
