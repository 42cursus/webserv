/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   State.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 20:47:27 by margo             #+#    #+#             */
/*   Updated: 2025/10/01 23:50:49 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef STATE_HPP
#define STATE_HPP

#include <map>
#include <vector>
#include <iostream>
#include <string>

/*
    STATES:
        1. parsing > should it be a state in itself?
        2. http/server/location etc.
        3. inside block/directive(s)
        4. errors (e.g. if config file doesn't exist/can't be read)
        5. comment
        6. quotes
*/

class Parser;
class   IBlock;

typedef struct Parameter
{
    std::string value;
    int rlidx;
} Parameter;

typedef struct Comment
{
    std::string content;
    int rlidx;
} Comment;

class IState
{
    public:
        virtual ~IState() {};
        
        virtual void    enter(Parser*   parser) = 0;
        virtual void    toggle(Parser*  parser) = 0;
        //virtual void    exit(Parser*  parser) = 0;
} ;

class   Start: public IState
{
    private:
        Start(const Start& copy);
        Start& operator=(const Start& copy);
    
        
    public:
        Start();
        ~Start();
        
        void    enter(Parser* parser);
        void    toggle(Parser *parser);
        void    exit(Parser* parser);
} ;

class   IBlock: public IState
{
    private:
        std::string     _name;
        int _line;
        std::string _code;
        IState* _parent;
        std::vector<Parameter> _parameters;
        std::vector<IState*> _directives;

        IBlock();
        IBlock(const IBlock& copy);
        IBlock& operator=(const IBlock& copy);

    public:
        IBlock(const std::string& name);
        bool operator==(const IBlock& oth);
        ~IBlock();

        int getLine() const;
        void    setLine(int line);
        std::string    getName() const;
        std::string    getCode() const;
        std::vector<IState*> getDirectives() const;
        std::vector<Parameter>  getParameters() const;
        void    addParameter(Parameter newParameter);
        void    addDirective(IState* newDir);
        IState* getParent() const;
        void    setParent(IState*   parentDirective);

        virtual void    enter(Parser* parser) = 0;
        virtual void    toggle(Parser* parser) = 0;
        virtual void    exit(Parser*    parser) = 0;

} ;

/*
    1. HTTP (context)
    2. server (context)
    //3. listen 
    4. location (context)
    5. root
    6. index
*/

class   HTTP: public IBlock
{
    private:
        HTTP(const HTTP& copy);
        HTTP& operator=(const HTTP& copy);
    
    public:
        HTTP();
        ~HTTP();

        void    enter(Parser* parser);
        void    toggle(Parser* parser);
        void    exit(Parser* parser);
        //void    storeCode();
} ;

class Server: public IBlock
{
    private:
        Server(const Server& copy);
        Server& operator=(const Server& copy);
        
    public:
        Server();
        ~Server();

        void    enter(Parser* parser);
        void    toggle(Parser* parser);
        void    exit(Parser* parser);
} ;

class Location: public IBlock
{
    private:
        Location(const Location& copy);
        Location& operator=(const Location& copy);
        std::string path_prefix;     // "/cgi-bin", "/static", "/"
        bool        is_cgi;
        bool        autoindex;
        std::string cgi_script;
        std::vector<> allowed_methods;
    public:
        Location();
        ~Location();

        void enter(Parser* parser);
        void toggle(Parser* parser);
        void exit(Parser* parser);
} ;

#endif
