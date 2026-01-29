/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 04:54:36 by abelov            #+#    #+#             */
/*   Updated: 2026/01/23 15:41:22 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Prefix_suffix.hpp"


/*
** -------------------------------- STATIC VARS -------------------------------
*/


/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

Parser::Parser(IState *currentState)
    : _currentState(currentState), _inBlock(false), _config() {
    Parser::memset(&_config, 0, sizeof(_config));
}

Parser::Parser() {
    Parser::memset(&_config, 0, sizeof(_config));
    _config = make_default_config();
}

Parser::Parser(const Parser &copy) {
    (void) copy;
}

Parser::errorException::errorException(const std::string &msg) : _errorMsg(msg) {};

const char *Parser::errorException::what() const throw() {
    return _errorMsg.c_str();
}
/*
** ------------------------------- DESTRUCTOR ---------------------------------
*/

/*
** -------------------------------- OPERATORS ---------------------------------
*/

bool Parser::Token::operator==(const Parser::Token &other) const {
    return (type == other.type && literal == other.literal && line == other.line);
}

bool Parser::operator==(const Parser &rhs) const {
    return _key == rhs._key &&
           _configRoot == rhs._configRoot &&
           _currentToken == rhs._currentToken &&
           _nextToken == rhs._nextToken &&
           _currentIt == rhs._currentIt &&
           _currentState == rhs._currentState &&
           _inBlock == rhs._inBlock &&
           _blocks == rhs._blocks &&
           commentBuf == rhs.commentBuf;
//           commentBuf == rhs.commentBuf &&
//           _config == rhs._config;
}

bool Parser::operator!=(const Parser &rhs) const {
    return !(rhs == *this);
}

/*
** --------------------------------- METHODS ----------------------------------
*/

std::string Parser::readQuotedString(std::string word) {
    if (word.length() >= 2 && word[0] == '"' && word[word.size() - 1] == '"')
        return word.substr(1, word.length() - 2);

    return word;
}

void	add_default_locations(Config &cfg)
{
	std::vector<Location> &locations = cfg.http.server.locations;
//	locations.push_back(upload);
//	locations.push_back(data);
	for (uint64_t i = 0; i < locations.size(); i++)
	{
		loc_trie_insert(cfg.http.server.loc_trie, &locations[i]);
	}
}

Config Parser::make_default_config() {
    const std::string index[] = {
        "index.html",
        "index.htm",
        ""};

    Config cfg = {
        .http = {
            .server = {
                .ipv4_listen = {
                    .sin_family = AF_INET,
                    .sin_port = htons(8080),
                    .sin_addr = {
//                        .s_addr = inet_addr("127.0.0.1")
                        .s_addr = htonl(INADDR_ANY)},
                    .sin_zero = {0x00}},
                .server_name = "localhost",
                .locations = std::vector<Location>(),
                .loc_trie = new TrieNode(),
                .error_pages = std::map<std::string, std::string>()}}
    };
    add_default_locations(cfg);
    return cfg;
}

std::string Parser::read_file(const char *filename) {
    //Config conf = make_default_config();
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Err: File doesn't exist or can't be opened." << std::endl;
        return std::string();
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}


Parser::Token Parser::makeToken(TokenType key, std::string word, int linecount) {
    Token token;

    token.type = key;
    token.literal = word;
    token.line = linecount;

    return token;
}

void Parser::toggle() {
    _currentState->toggle(this);
}

std::vector<Parser::Token> Parser::tokenize() {
    int linecount = 0;
    std::string line;
    std::vector<Parser::Token> tokens;

    std::ifstream file(_configRoot.c_str());
    if (!file.is_open()) {
        std::cerr << "Err: File doesn't exist or can't be opened." << std::endl;
        return std::vector<Parser::Token>();
    }

    while (std::getline(file, line)) {
        linecount++;
        std::istringstream iss(line);
        std::string word;

        if (line.empty())
            continue;

        while (iss >> word) {
            if (word[0] == '#') {
                tokens.push_back(makeToken(COMMENT, word, linecount));// to do: function for storing comments
                break;
            }
            if (word == "http" || word == "server" || word == "listen" || word == "location" || word == "root" || word == "index")
                tokens.push_back(makeToken(KEY, word, linecount));
            else if (word == "{")
                tokens.push_back(makeToken(BLOCK_START, word, linecount));
            else if (word == "}")
                tokens.push_back(makeToken(BLOCK_END, word, linecount));
            else if (word.find('/') != std::string::npos)
                tokens.push_back(makeToken(REGEX, word, linecount));
            else if (word[0] == '"' && word[word.length() - 1] == '"')
                tokens.push_back(makeToken(QUOTES, readQuotedString(word), linecount));
            else if (word[0] == '$')
                tokens.push_back(makeToken(VAR, word, linecount));// to do: function extracting/expanding variable ???
            else if (word == ";" || word[word.length() - 1] == ';') {
                if (word != ";")
                    tokens.push_back(makeToken(KEY, word.substr(0, word.length() - 1), linecount));
                tokens.push_back(makeToken(SEMICOLON, ";", linecount));
            } else if (word[word.length() - 1] == '\n' || word == "\n") {
                if (word != "\n")
                    tokens.push_back(makeToken(KEY, word.substr(0, word.length() - 1), linecount));
                tokens.push_back(makeToken(EOL, "\n", linecount));
            } else
                tokens.push_back(makeToken(ILLEGAL, word, linecount));
        }
    }

    file.close();
    return tokens;
}

std::map<std::string, std::string> Parser::init_mime_types() {
    std::ifstream file("./mime.types");

    if (!file.is_open()) {
        std::cerr << "Err: mime.types can't be opened or doesn't exist." << std::endl;
        return std::map<std::string, std::string>();
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string mtype;
        if (line.find(';') == std::string::npos)
            continue;                                // skips lines without semicolons
        size_t start = line.find_first_not_of(" \t");// checks for whitespace/indentation
        size_t end = 0;
        if (start != 0) {
            // extract substr and store them in map
            std::istringstream iss(line);
            std::string word;
            while (iss >> word) {
                end = word.length();
                mtype = line.substr(start, end);
                std::cout << mtype << std::endl;
            }
        }
    }
    return (std::map<std::string, std::string>());
}


void Parser::printTokens(std::vector<Parser::Token> tokens) {
    for (std::vector<Parser::Token>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
        if (it->type == EOL)
            std::cout << "EOL " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == KEY)
            std::cout << "KEYWORD " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == VAR)
            std::cout << "VARIABLE " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == QUOTES)
            std::cout << "QUOTED STRING " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == BLOCK_START)
            std::cout << "BLOCK START " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == BLOCK_END)
            std::cout << "BLOCK END " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == SEMICOLON)
            std::cout << "SEMICOLON " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == REGEX)
            std::cout << "REGEX " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == COMMENT)
            std::cout << "COMMENT " << it->literal << " on line " << it->line << std::endl;
        else if (it->type == ILLEGAL)
            std::cout << "ILLEGAL " << it->literal << " on line " << it->line << std::endl;
        else
            std::cout << "UNKNOWN " << it->literal << " on line " << it->line << std::endl;
    }
}

void Parser::parseServer(std::vector<Parser::Token> &tokens) {
    std::vector<Parser::Token>::iterator it;

    _config.http.server.ipv4_listen.sin_family = AF_INET;
    _config.http.server.ipv4_listen.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    memset(_config.http.server.ipv4_listen.sin_zero, 0, sizeof(_config.http.server.ipv4_listen.sin_zero));
    for (it = _currentIt; it != tokens.end(); ++it) {
        if (it->literal == "listen" && (it + 1) != tokens.end()) {
            int port;
            std::stringstream ss((it + 1)->literal);
            ss >> port;
            if (port < 1 || port > UINT16_MAX)
                throw errorException("Invalid config: port");
            _config.http.server.ipv4_listen.sin_port = htons(port);
            ++it;
        }
        if (it->literal == "server_name" && (it + 1) != tokens.end()) {
            _config.http.server.server_name = (it + 1)->literal;
            //std::cout << _config.http.server.server_name << std::endl;
            ++it;
        }
    }
}

void Parser::parse(std::vector<Parser::Token> &tokens) {
    std::vector<Parser::Token>::iterator it;

    setCurrentState(new Start());
    for (it = tokens.begin(); it != tokens.end(); ++it) {
        _currentIt = it;
        _currentToken = *it;
        if (it + 1 != tokens.end())
            _nextToken = *(it + 1);

        //std::cout << getCurrentToken().literal << ", " << getCurrentToken().line << std::endl;
        toggle();
        if (dynamic_cast<Server *>(_currentState) != NULL) {
            // populate Config struct w sockaddr_in and server_name
            parseServer(tokens);
            it = _currentIt;
        } else if (dynamic_cast<Location *>(_currentState) != NULL) {
            // populate Config struct w path
            toggle();
            it = _currentIt;
        }
        /*
        else if (dynamic_cast<locConfig*>(_currentState) != NULL)
        {
            // populate Config struct w root and index
        }
        */
    }
}


std::string Parser::getFullLine(int line) {
    return std::string();
    (void) line;
}

std::string Parser::readUntil(std::string line, char delim) {
    return std::string();
    (void) line;
    (void) delim;
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

/*

		TO DO:
			//1. open and read config file (copy in buf stream and close)
			2. read http
			//3. read server
			//4. read listening port > hostname/IP and TCP port
			5. read config stuff (root +  index + anything else)
			6. construct config struct
 */


void Parser::setCurrentState(IState *newState) {
    _currentState = newState;
}


IBlock *Parser::getBlock(std::string key) {
    for (std::vector<IBlock *>::iterator it = _blocks.begin(); it != _blocks.end(); ++it) {
        if ((*it)->getName() == key)
            return *it;
    }

    return NULL;
}

void Parser::memset(void *s, int c, size_t n) __THROW {
    std::memset(s, c, n);
}

/*
** -------------------------------- TESTS ---------------------------------
*/


void printConfig(Config cfg) {
    uint16_t port;
    std::string locationPath;
    std::string configRoot;
    std::string server_name;

    sockaddr_in &in = cfg.http.server.ipv4_listen;
    port = ntohs(in.sin_port);

    server_name = cfg.http.server.server_name;
//    locationPath = cfg.http.server.location.path;
//    configRoot = cfg.http.server.location.config.root;

    char buf[INET_ADDRSTRLEN];
    const char *addr = inet_ntop(AF_INET, &in.sin_addr, static_cast<char *>(buf), INET_ADDRSTRLEN);

    std::cout << "Address Family: " << static_cast<int>(in.sin_family) << "\n"
              << "Port: " << port << "\n"
              << "IP Address: " << addr << "\n"
              << "Server Name: " << (server_name.empty() ? server_name : "NULL") << "\n"
              << "Location Path: " << locationPath << "\n"
              << "Config Root: " << configRoot << "\n"
              << "Index Files: ";

//    for (int i = 0; !cfg.http.server.location.config.index[i].empty(); i++) {
//        std::cout << cfg.http.server.location.config.index[i];
//        if (!cfg.http.server.location.config.index[i + 1].empty())
//            std::cout << ", ";
//    }
    std::cout << std::endl;
}

/*
	TO DO:
		1. finish parsing function
		//2. function to print Config struct for testing purposes
		3. update Config to be able to handle multiple servers
*/

/*
int	main(int argc, char *argv[])
{
    Parser*	parse = new Parser();
    std::vector<Parser::Token> tokens;
    const char *config_root = "../../resources/webserv.conf";

    if (argc == 2)
        config_root = argv[1];
    parse->setConfigRoot(config_root);
    tokens = parse->tokenize();
    //printTokens(tokens);
    parse->parse(tokens);
    printConfig(parse->_config);
}
*/
