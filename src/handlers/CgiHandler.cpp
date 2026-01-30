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
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Prefix_suffix.hpp"
#include "serve.hpp"
#include "webserv.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/epoll.h>
#include <sstream>

#include <cstdio>
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

/*
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

*/


/*
** -------------------------------- STATIC VARS -------------------------------
*/

namespace {
//    std::string trim(const std::string &s);
//    bool starts_with(const std::string &s, const std::string &prefix);
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

StatusCode CgiHandler::handle(HttpRequest &req, HttpResponse &res)
{
    pipe(_stdin_pipe);
    pipe(_stdout_pipe);


    this->_pid = fork();
    if (_pid == 0) // child
    {
        dup2(_stdout_pipe[1], STDOUT_FILENO); // stdout -> pipe
        close(_stdout_pipe[0]);
        close(_stdout_pipe[1]);

        dup2(_stdin_pipe[0], STDIN_FILENO); // pipe -> stdin
        close(_stdin_pipe[1]);
        close(_stdin_pipe[0]);

        // build ENVP

        std::string script = apply_location(req.path, res.location);
        // build ARGV


        const char *argv[3] = {
            "/usr/bin/python3",
            script.c_str(),
            NULL,
        };
        const char *envp[3] = {
            "FUCK=me",
            "TWAT=you",
            NULL,
        };
        execve(argv[0], (char *const *)argv, (char *const *)envp);
    }
    else if (_pid < 0)
    {
        return (SC_500);
    }

    close(_stdout_pipe[1]);
    close(_stdin_pipe[0]);

    // write(_stdin_pipe[1], "", 0);
    // close(_stdin_pipe[1]);

    // FILE*	fp = fdopen(_stdout_pipe[0], "r");
    // char	*line = NULL;
    // size_t	n = 0;
    // ssize_t	nread = 0;
    // while ((nread = getline(&line, &n, fp)) != -1)
    // {
    //     res.body.append(line);
    // }
    // fclose(fp);
    // free(line);
    // wait(NULL);
	
	
    return SC_200;
}

void	CgiHandler::register_read_pipe(int epoll_fd)
{
	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.data.ptr = tag_ptr(this, EP_CGIS);
	ev.events = EPOLLIN;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->_stdout_pipe[0], &ev);
}

void	CgiHandler::register_write_pipe(int epoll_fd)
{
	struct epoll_event ev;
	std::memset(&ev, 0, sizeof(ev));
	ev.data.ptr = tag_ptr(this, EP_CGIS);
	ev.events = EPOLLOUT;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->_stdin_pipe[1], &ev);
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

/*
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

*/
