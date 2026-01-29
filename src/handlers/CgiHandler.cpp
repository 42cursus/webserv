/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 02:09:33 by abelov            #+#    #+#             */
/*   Updated: 2026/01/20 14:42:02 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



CgiHandler::CgiHandler(HttpRequest& req, const Location& loc, const std::string& script_path, HttpResponse& res) :
    _state(), _pid(0),
    _stdin_pipe(), _stdout_pipe(),
     _res(res), _req(req),
    _script_path(script_path)
{
    (void)loc;
}

void CgiHandler::_build_env(std::vector<std::string>& env)
{
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");

    env.push_back("REQUEST_METHOD=" + _req.method);
    env.push_back("SCRIPT_NAME=" + _req.path);
    env.push_back("PATH_INFO=" + _req.path);

    if (_req.headers.count("query-string"))
        env.push_back("QUERY_STRING=" + _req.headers["query-string"]);
    else
        env.push_back("QUERY_STRING=");

    if (_req.headers.count("content-type"))
        env.push_back("CONTENT_TYPE=" + _req.headers["content-type"]);
    else
        env.push_back("CONTENT_TYPE=");

    if (_req.headers.count("content-length"))
        env.push_back("CONTENT_LENGTH=" + _req.headers["content-length"]);
    else
        env.push_back("CONTENT_LENGTH=0");
}

std::string	CgiHandler::raw_output(void) const
{
    return (_raw_output);
}

void CgiHandler::_parse_output_into_response()
{
    // Split headers/body at first empty line. Accept \r\n\r\n or \n\n.
    size_t sep = _raw_output.find("\r\n\r\n");
    size_t sep_len = 4;
    if (sep == std::string::npos)
    {
        sep = _raw_output.find("\n\n");
        sep_len = 2;
    }

    std::string header_block;
    std::string body;
    if (sep == std::string::npos)
    {
        // No CGI headers -> treat everything as body.
        header_block = "";
        body = _raw_output;
    }
    else
    {
        header_block = _raw_output.substr(0, sep);
        body = _raw_output.substr(sep + sep_len);
    }

    _res.body = body;

    if (_res.headers.find("content-type") == _res.headers.end())
        _res.headers["content-type"] = "text/plain";

    std::istringstream iss(header_block);
    std::string line;
    while (std::getline(iss, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        if (starts_with(line, "Status:"))
        {
            std::string v = trim(line.substr(std::strlen("Status:")));
            // format: "200 OK"
            std::istringstream ss(v);
            int code = 200;
            ss >> code;
            _res.statuscode = ::itoa(code);

            std::string rest;
            std::getline(ss, rest);
            rest = trim(rest);
            _res.statusmsg = rest.empty() ? "OK" : rest;
            continue;
        }

        size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        std::string key = line.substr(0, colon);
        std::string val = trim(line.substr(colon + 1));

        // normalize header key to lowercase (your codebase uses lowercase keys)
        for (size_t i = 0; i < key.size(); i++)
            key[i] = static_cast<char>(std::tolower(key[i]));

        _res.headers[key] = val;
    }
}




/*
** -------------------------------- STATIC VARS -------------------------------
*/

namespace {
    std::string trim(const std::string &s);
    bool starts_with(const std::string &s, const std::string &prefix);
}

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/


/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

int CgiHandler::handle(HttpRequest &req, HttpResponse &res) {
    // Create pipes: stdin_pipe (parent writes -> child reads), stdout_pipe (child writes -> parent reads)
    if (pipe(_stdin_pipe) != 0)
        return 500;
    if (pipe(_stdout_pipe) != 0)
        return 500;

    _pid = fork();
    if (_pid < 0)
        return 500;

    if (_pid == 0)
    {
        // child
        dup2(_stdin_pipe[0], STDIN_FILENO);
        dup2(_stdout_pipe[1], STDOUT_FILENO);

        close(_stdin_pipe[0]);
        close(_stdin_pipe[1]);
        close(_stdout_pipe[0]);
        close(_stdout_pipe[1]);

        std::vector<std::string> env;
        _build_env(env);

        std::vector<char*> envp;
        envp.reserve(env.size() + 1);
        for (size_t i = 0; i < env.size(); i++)
            envp.push_back(const_cast<char*>(env[i].c_str()));
        envp.push_back(NULL);

        // argv[0] is script path
        // NOTE: script_path is stored in caller; we pass it as argv[0] here.
        // execve requires a writable char* array; const_cast is ok for argv/envp.
        // The kernel won't modify your strings.
        // ...
        // The actual script path is passed via SCRIPT_NAME / PATH_INFO etc.
        // ...
        // IMPORTANT: the script must be executable (chmod +x).
        // ...
        // If execve fails, exit non-zero.
        char *argv[2];
        argv[0] = const_cast<char*>(_script_path.c_str());
        argv[1] = NULL;

        execve(argv[0], argv, &envp[0]);
        _exit(127);
    }

    // parent
    close(_stdin_pipe[0]);
    close(_stdout_pipe[1]);

    // Write request body to CGI stdin (for POST); for GET this is empty.
    if (!_req.body.empty())
    {
        size_t off = 0;
        while (off < _req.body.size())
        {
            ssize_t w = write(_stdin_pipe[1], &_req.body[off], _req.body.size() - off);
            if (w <= 0)
                break;
            off += static_cast<size_t>(w);
        }
    }
    close(_stdin_pipe[1]); // EOF for CGI stdin

    // Read all CGI stdout
    _raw_output.clear();
    char buf[4096];
    while (true)
    {
        ssize_t r = read(_stdout_pipe[0], buf, sizeof(buf));
        if (r <= 0)
            break;
        _raw_output.append(buf, static_cast<size_t>(r));
    }
    close(_stdout_pipe[0]);

    int status = 0;
    waitpid(_pid, &status, 0);

    // Default response if CGI didn't set anything
    if (_res.statuscode.empty())
    {
        _res.statuscode = "200";
        _res.statusmsg = "OK";
    }

    _parse_output_into_response();

    // Ensure content-length matches the body we extracted (even if script didn't set it)
    _res.headers["content-length"] = ::itoa(static_cast<int>(_res.body.size()));
    return std::atoi(_res.statuscode.c_str());
}

/*
** --------------------------------- METHODS ----------------------------------
*/

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

HttpResponse &CgiHandler::res() const {
    return _res;
}

HttpRequest &CgiHandler::req() const {
    return _req;
}

std::string CgiHandler::body_buffer() const {
    return _body_buffer;
}

/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/

namespace {
    bool starts_with(const std::string &s, const std::string &prefix) {
        return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
    }


    std::string trim(const std::string &s) {
        size_t b = 0;
        while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n'))
            b++;
        size_t e = s.size();
        while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n'))
            e--;
        return s.substr(b, e - b);
    }
}

