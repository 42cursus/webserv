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
class IDirective;
class   IBlock;

typedef struct Parameter
{
    std::string value;
    int rlidx;
} Parameter;

class IState
{
    public:
        virtual ~IState() {};
        
        virtual void    enter(Parser*   parser) = 0;
        virtual void    toggle(Parser*  parser) = 0;
        virtual void    exit(Parser*  parser) = 0;
} ;

class   IBlock: public IState
{
    private:
        std::string     _name;
        std::string _code;
        IDirective* _parent;
        std::vector<IDirective*> _directives;

        IBlock();
        IBlock(const IBlock& copy);
        IBlock& operator=(const IBlock& copy);

    public:
        IBlock(std::string name);
        ~IBlock();

        std::string    getName() const;
        std::string    getCode() const;
        std::vector<IDirective*> getDirectives() const;
        IDirective* getParent() const;
        void    setParent(IDirective*   parentDirective);

        void    enter(Parser* parser);
        void    toggle(Parser* parser);
        void    exit(Parser*    parser);

} ;

class   IDirective: public IState
{
    private:
        std::string _name;
        int _line;
        std::vector<Parameter> _parameters;
        std::vector<IDirective*> _directives;
        IBlock* _block;
        IDirective* _parent;

        IDirective();
        IDirective(const IDirective* copy);
        IDirective& operator=(const IDirective* copy);

    public:
        IDirective(std::string name);
        ~IDirective();

        std::string getName() const;
        std::vector<Parameter>  getParameters() const;
        std::vector<IDirective*> getDirectives() const;
        IBlock* getBlock() const;
        IDirective* getParent() const;
        void    setParent(IDirective* parentDirective);
        int getLine() const;
        void    setLine(int line);

        void    enter(Parser* parser);
        void    toggle(Parser* parser);
        void    exit(Parser*    parser);
} ;

class   HTTP: public IBlock
{
    private:
        HTTP(const HTTP& copy);
        HTTP& operator=(const HTTP& copy);
    
    public:
        HTTP();
        ~HTTP();
} ;

#endif