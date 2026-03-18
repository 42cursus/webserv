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
#include "Connection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Prefix_suffix.hpp"
#include "WebServer.hpp"
#include "webserv.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <sys/epoll.h>

#include <cstdio>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


/*
** -------------------------------- STATIC VARS -------------------------------
*/

namespace {
	std::string trim(const std::string &s);
	bool		starts_with(const std::string &s, const std::string &prefix);
}// namespace

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

CgiHandler::CGISession::CGISession() :
	_pid(0),
	_stdin_pipe(),
	_stdout_pipe(),
	bytes_sent(0),
	bytes_received(0),
	_stdinClosed(false)
{}

CgiHandler::CgiHandler(HttpRequest		 &req,
					   const Location	 &loc,
					   const std::string &script_path,
					   HttpResponse		 &res) :
	_state(),
	_res(res),
	_req(req),
	_script_path(script_path)
{
	(void) loc;
}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/


/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

StatusCode CgiHandler::handle(HttpRequest &req, HttpResponse &res)
{
	CGISession sess;
	sess._parentConnection = const_cast<Connection *>(this->wrkr->getConnPtr());
	pipe(sess._stdin_pipe);
	pipe(sess._stdout_pipe);

	sess._pid = fork();
	if (sess._pid == 0)// child
	{
		dup2(sess._stdout_pipe[1], STDOUT_FILENO);// stdout -> pipe
		close(sess._stdout_pipe[0]);
		close(sess._stdout_pipe[1]);

		dup2(sess._stdin_pipe[0], STDIN_FILENO);// pipe -> stdin
		close(sess._stdin_pipe[1]);
		close(sess._stdin_pipe[0]);

		// build ENVP

		std::string script = apply_location(req.path, res.location);
		// build ARGV

		std::vector<std::string> env;
		_build_env(env);
		env.push_back("TRY=me");
		env.push_back("SEE=you");

		std::vector<char*> envp;
		envp.reserve(env.size() + 1);
		for (size_t i = 0; i < env.size(); ++i)
			envp.push_back(::strdup(env[i].c_str()));
		envp.push_back(NULL);

		std::vector<std::string> argv_str;
		argv_str.push_back("/usr/bin/python3");
		argv_str.push_back(script);

		std::vector<char*> argv;
		argv.reserve(argv_str.size() + 1);
		for (size_t i = 0; i < argv_str.size(); ++i) {
			const std::string &str = argv_str[i];
			char *buf = new char[str.size() + 1]; // allocating RAW memory
			std::strncpy(buf, str.c_str(), str.size() + 1);
			argv.push_back(buf);
		}
		argv.push_back(NULL);
		if (execve(argv[0], &argv[0], &envp[0]) == -1) {
			for (size_t i = 0; i < argv.size(); ++i)
				delete[] argv[i];
			for (size_t i = 0; i < envp.size(); ++i)
				delete[] envp[i];

			std::exit(EXIT_FAILURE); // probably should be 127
		}
	} else if (sess._pid < 0)
		return (SC_500);

	close(sess._stdout_pipe[1]);
	close(sess._stdin_pipe[0]);
	set_non_blocking(sess._stdin_pipe[1]);
	set_non_blocking(sess._stdout_pipe[0]);

	// write(sess._stdin_pipe[1], "", 0);
	// close(sess._stdin_pipe[1]);

	// FILE*	fp = fdopen(_stdout_pipe[0], "r");
	// char	*line = NULL;
	// size_t	n = 0;
	// ssize_t	nread = 0;
	// while ((nread = getline(&line, &n, fp)) != -1)
		// res.body.append(line);
	// fclose(fp);
	// free(line);

	// FIXME: do it in a non-blocking way
	// waitpid(sess._pid, &sess._wstatus, WUNTRACED);
	// while (!WIFEXITED(sess._wstatus) && !WIFSIGNALED(sess._wstatus))
	// 	waitpid(sess._pid, &sess._wstatus, WUNTRACED);

	CGISession *session = new CGISession();
	*session = sess;
	session->stageRequestBody(req.body);
	this->wrkr->setCgiSession(session);
	return SC_200;
	(void)_state;
}

StatusCode CgiHandler::handlePHP(HttpRequest &req, HttpResponse &res)
{
	CGISession sess;
	sess._parentConnection = const_cast<Connection *>(this->wrkr->getConnPtr());
	pipe(sess._stdin_pipe);
	pipe(sess._stdout_pipe);

	sess._pid = fork();
	if (sess._pid == 0)// child
	{
		dup2(sess._stdout_pipe[1], STDOUT_FILENO);// stdout -> pipe
		close(sess._stdout_pipe[0]);
		close(sess._stdout_pipe[1]);

		dup2(sess._stdin_pipe[0], STDIN_FILENO);// pipe -> stdin
		close(sess._stdin_pipe[1]);
		close(sess._stdin_pipe[0]);

		// build ENVP

		std::string script = apply_location(req.path, res.location);
		// build ARGV

		std::vector<std::string> env;
		_build_env(env);
		env.push_back("TRY=me");
		env.push_back("SEE=you");
		env.push_back("REDIRECT_STATUS=200");

		std::vector<char*> envp;
		envp.reserve(env.size() + 1);
		for (size_t i = 0; i < env.size(); ++i)
			envp.push_back(::strdup(env[i].c_str()));
		envp.push_back(NULL);

		std::vector<std::string> argv_str;
		argv_str.push_back("/usr/bin/php-cgi");
		argv_str.push_back(script);

		std::vector<char*> argv;
		argv.reserve(argv_str.size() + 1);
		for (size_t i = 0; i < argv_str.size(); ++i) {
			const std::string &str = argv_str[i];
			char *buf = new char[str.size() + 1]; // allocating RAW memory
			std::strncpy(buf, str.c_str(), str.size() + 1);
			argv.push_back(buf);
		}
		argv.push_back(NULL);
		if (execve(argv[0], &argv[0], &envp[0]) == -1) {
			for (size_t i = 0; i < argv.size(); ++i)
				delete[] argv[i];
			for (size_t i = 0; i < envp.size(); ++i)
				delete[] envp[i];

			std::exit(EXIT_FAILURE); // probably should be 127
		}
	} else if (sess._pid < 0)
		return (SC_500);

	close(sess._stdout_pipe[1]);
	close(sess._stdin_pipe[0]);
	set_non_blocking(sess._stdin_pipe[1]);
	set_non_blocking(sess._stdout_pipe[0]);

	// write(sess._stdin_pipe[1], "", 0);
	// close(sess._stdin_pipe[1]);

	// FILE*	fp = fdopen(_stdout_pipe[0], "r");
	// char	*line = NULL;
	// size_t	n = 0;
	// ssize_t	nread = 0;
	// while ((nread = getline(&line, &n, fp)) != -1)
		// res.body.append(line);
	// fclose(fp);
	// free(line);

	// FIXME: do it in a non-blocking way
	// waitpid(sess._pid, &sess._wstatus, WUNTRACED);
	// while (!WIFEXITED(sess._wstatus) && !WIFSIGNALED(sess._wstatus))
	// 	waitpid(sess._pid, &sess._wstatus, WUNTRACED);

	CGISession *session = new CGISession();
	*session = sess;
	session->stageRequestBody(req.body);
	this->wrkr->setCgiSession(session);
	return SC_200;
	(void)_state;
}

Connection::e_result CgiHandler::CGISession::onWritable()
{
	static const size_t kPipeWriteChunk = 8192;
	if (_stdinClosed)
		return Connection::OK;

	if (_stdinBuckets.bytes() == 0) {
		close(_stdin_pipe[1]);
		_stdinClosed = true;
		return Connection::OK;
	}

	std::string payload = _stdinBuckets.flatten(kPipeWriteChunk);
	if (payload.empty())
		return Connection::WANT_WRITE;

	const ssize_t written = write(_stdin_pipe[1], payload.data(), payload.size());
	if (written > 0) {
		_stdinBuckets.consume(static_cast<size_t>(written));
		bytes_sent += static_cast<size_t>(written);
		if (_stdinBuckets.bytes() == 0) {
			close(_stdin_pipe[1]);
			_stdinClosed = true;
			return Connection::OK;
		}
		return Connection::WANT_WRITE;
	}
	if (written < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
		return Connection::WANT_WRITE;

	return Connection::ERROR;
}

Connection::e_result CgiHandler::CGISession::onReadable()
{
	Connection::e_result result = Connection::OK;
	std::vector<char> v(8192);

	const ssize_t bytesRead = read(this->_stdout_pipe[0], &v[0], v.size());
	HttpResponse *res = _parentConnection->getCurrentResponse();
	if (res == NULL)
		return Connection::ERROR;

	if (bytesRead > 0) {
		v.resize(bytesRead);
		_stdoutBuckets.appendMemory(&v[0], static_cast<size_t>(bytesRead));
		std::string streamed = _stdoutBuckets.flatten(0);
		if (!streamed.empty()) {
			_raw_output += streamed;
			_body_buffer += streamed;
			res->body.append(streamed);
			_stdoutBuckets.consume(streamed.size());
		}
		bytes_received += static_cast<size_t>(bytesRead);

		result = Connection::WANT_WRITE;
	} else if (bytesRead == 0) {
		_stdoutBuckets.appendEOS();
		res->body_complete = true;
		result = Connection::OK;
	} else if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
		result = Connection::OK;
	} else {
		result = Connection::ERROR;
	}
	return result;
}

int CgiHandler::CGISession::register_read_pipe(int epoll_fd)
{
	struct epoll_event ev = {};
	ev.data.ptr = tag_ptr(this, WebServer::EP_CGI);
	ev.events	= EPOLLIN;
	return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _stdout_pipe[0], &ev);
}

int CgiHandler::CGISession::register_write_pipe(int epoll_fd)
{
	struct epoll_event ev = {};
	CGISession *ptr = this;
	ev.data.ptr = tag_ptr(ptr, WebServer::EP_CGI);
	ev.events	= EPOLLOUT;
	return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _stdin_pipe[1], &ev);
	return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->_parentConnection->getFd(), &ev);
}

void CgiHandler::CGISession::stageRequestBody(const std::vector<char> &body)
{
	_stdinBuckets.clear();
	if (!body.empty())
		_stdinBuckets.appendMemory(&body[0], body.size());
	_stdinBuckets.appendEOS();
}

void CgiHandler::_build_env(std::vector<std::string> &env)
{
	env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");

	env.push_back("REQUEST_METHOD=" + _req.method);
	env.push_back("SCRIPT_NAME=" + _req.path);
	env.push_back("PATH_INFO=");

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
		env.push_back("CONTENT_LENGTH=");
}

void CgiHandler::_parse_output_into_response(CGISession &sess)
{
	// Split headers/body at first empty line. Accept \r\n\r\n or \n\n.
	size_t sep	   = sess._raw_output.find(CRLF CRLF);
	size_t sep_len = 4;
	if (sep == std::string::npos) {
		sep		= sess._raw_output.find("\n\n");
		sep_len = 2;
	}

	std::string header_block;
	std::string body;
	if (sep == std::string::npos) {
		// No CGI headers -> treat everything as body.
		header_block = "";
		body		 = sess._raw_output;
	} else {
		header_block = sess._raw_output.substr(0, sep);
		body		 = sess._raw_output.substr(sep + sep_len);
	}

	_res.body = body;

	if (_res.headers.find("content-type") == _res.headers.end())
		_res.headers["content-type"] = "text/plain";

	std::istringstream iss(header_block);
	std::string		   line;
	while (std::getline(iss, line)) {
		line = trim(line);
		if (line.empty())
			continue;

		if (starts_with(line, "Status:")) {
			std::string v = trim(line.substr(std::strlen("Status:")));
			// format: "200 OK"
			std::istringstream ss(v);
			int				   code = SC_200;
			ss >> code;
			_res.statuscode = ::itoa(code);

			std::string rest;
			std::getline(ss, rest);
			rest		   = trim(rest);
			_res.statusmsg = rest.empty() ? "OK" : rest;
			continue;
		}

		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = line.substr(0, colon);
		std::string val = trim(line.substr(colon + 1));

		// normalize a header key to lowercase (your codebase uses lowercase keys)
		for (size_t i = 0; i < key.size(); i++)
			key[i] = static_cast<char>(std::tolower(key[i]));

		_res.headers[key] = val;
	}
}

/*
** -------------------------------- ACCESSORS ---------------------------------
*/

HttpResponse &CgiHandler::res() const
{
	return _res;
}

HttpRequest &CgiHandler::req() const
{
	return _req;
}

std::string CgiHandler::CGISession::body_buffer() const
{
	return _body_buffer;
}

std::string CgiHandler::CGISession::raw_output() const
{
	return (_raw_output);
}

int CgiHandler::set_non_blocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return -1;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/

namespace {
	bool starts_with(const std::string &s, const std::string &prefix)
	{
		return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
	}


	std::string trim(const std::string &s)
	{
		size_t b = 0;
		while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n'))
			b++;
		size_t e = s.size();
		while (e > b &&
			   (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n'))
			e--;
		return s.substr(b, e - b);
	}
}// namespace
