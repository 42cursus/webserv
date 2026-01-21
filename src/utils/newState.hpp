/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newState.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 17:45:42 by margo             #+#    #+#             */
/*   Updated: 2026/01/21 20:11:37 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NEWSTATE_HPP
#define NEWSTATE_HPP

#include <iostream>
#include <vector>
#include <map>
#include <cstdint>
#include "newParser.hpp"

enum    e_block_type
{
    SERVER_,
    LOCATION_,
    CGI_,
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
        unsigned int    _line_start;
        unsigned int    _line_end;
        e_block_type    _type;
        IBlock* _parent_block; // if NULL we're in main server block
        std::vector<Directive> _directives;
        
        IBlock();
    public:
        IBlock(e_block_type type);
        IBlock(const IBlock& copy);
        IBlock& operator=(const IBlock& copy);
        bool    operator==(IBlock& oth);
        ~IBlock();

        // getters
        unsigned int    getStartLine() const { return _line_start; };
        unsigned int    getEndLine() const { return _line_end; };
        e_block_type    getBlockType() const { return _type; };
        IBlock* getParent() const { return _parent_block; };
        std::vector<Directive>  getDirectives() { return _directives; };
        
        // setters
        void    setStartLine(unsigned int line_start) { _line_start = line_start; };
        void    setEndLine(unsigned int line_end) { _line_end = line_end; };
        void    setParent(IBlock*   parent) { _parent_block = parent; };
        void    addDirective(Directive new_directive) { _directives.push_back(new_directive); };
        
        virtual void    start(Parser& parser) = 0;
        virtual void    toggle(Parser& parser) = 0;
        virtual void    exit(Parser& parser) = 0;
} ;

struct  CGI: public IBlock
{
    std::string _ext;
    std::string _script;
    
    void    start(Parser& parser);
    void    toggle(Parser& parser);
    void    exit(Parser& parser);
} ;

struct  Location: public IBlock
{  
    std::string _root; // where it's getting redirected;
    std::string _path; // the redirect 
    std::vector<std::string>    _index;
    std::vector<std::string>    _methods;
    uint64_t   _max_body_size;
    bool    _autoindex;
    std::vector<CGI>    _cgi;

    void    start(Parser& parser);
    void    toggle(Parser& parser);
    void    exit(Parser& parser);
} ;

class   Server: public IBlock
{
    private:
        unsigned int _port;
        std::string _hostname;
        std::vector<Location>   _locations;
        std::map<std::string, std::string> _error_pages; // map<error code, path to html>

    public:
        Server();
        Server(const Server& copy);
        Server& operator=(const Server& copy);
        ~Server();
        
        void    start(Parser& parser);
        void    toggle(Parser& parser);
        void    exit(Parser& parser);
} ;

#endif