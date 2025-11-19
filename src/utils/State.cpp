/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   State.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 20:47:27 by margo             #+#    #+#             */
/*   Updated: 2025/10/01 23:50:49 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "State.hpp"
#include "Parser.hpp"

Start::Start() {};

Start::~Start() {};

void    Start::enter(Parser* parser)
{
    (void)parser;
};

void    Start::toggle(Parser *parser)
{
    if (parser->getCurrentToken().literal == "http")
    {
        parser->setCurrentState(new HTTP());
        parser->getCurrentState()->enter(parser);
    }
    // else if for events or other top level directives if needed
}

void    Start::exit(Parser* parser)
{
    (void)parser;
}

IBlock::IBlock(const std::string& name) : _name(name), _line(0), _parent(NULL)
{
};

bool IBlock::operator==(const IBlock& oth)
{
    return this->getName() == oth.getName();     
}

IBlock::~IBlock() 
{
    delete _parent;

    for (std::vector<IState*>::iterator it = _directives.begin(); it != _directives.end(); ++it)
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

void    IBlock::addParameter(Parameter newParameter)
{
    _parameters.push_back(newParameter);
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

void    Server::toggle(Parser *parser)
{
    if (parser->getCurrentToken().literal == "location")
    {
        Location* location = new Location();
        location->setParent(this);
        parser->setCurrentState(location);
        exit(parser);
        parser->getCurrentState()->enter(parser);
    }
}

void    Server::exit(Parser *parser)
{
    addDirective(parser->getCurrentState());
}

Location::Location(): IBlock("location") {};

Location::~Location() {};

void Location::enter(Parser* parser)
{
    Parameter root;
    
    parser->setKey(getName());
    parser->setInBlock(false);
    setLine(parser->getCurrentToken().line);
    root.value = parser->getNextToken().literal;
    root.rlidx = parser->getNextToken().line;
    addParameter(root);
    parser->_config.http.server.location.path = strDupForConstChar(root.value.c_str());
}

void    Location::toggle(Parser *parser)
{
    if (parser->getCurrentToken().literal == "root")
    {
        const char *rootPath = strDupForConstChar(parser->getNextToken().literal.c_str());
        parser->_config.http.server.location.config.root = strDupForConstChar(rootPath);
    }
    else if (parser->getCurrentToken().literal == "index")
    {
        char *indexBuf = strDupForConstChar(parser->getNextToken().literal.c_str());
        std::vector<std::string>indexArr = parser->_config.http.server.location.config.index;

        int i = 0;
        while (strnCmp(indexBuf, ";", 2) != 0)
        {
            indexArr[i] = strDupForConstChar(indexBuf);
            i++;
            parser->toggleCurrentToken();
            indexBuf = strDupForConstChar(parser->getNextToken().literal.c_str());  
        }
    }
}

void    Location::exit(Parser* parser)
{
    addDirective(parser->getCurrentState());
}