/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 01:47:47 by abelov            #+#    #+#             */
/*   Updated: 2026/01/30 04:18:23 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <vector>

#include "Connection.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "IHandler.hpp"
#include "Location.hpp"

class ConnWorker;

class CgiHandler : public IHandler {
public:
	enum State { READY, WRITING_BODY, READING_OUTPUT, DONE, ERROR };

	class CGISession {
	public:
		CGISession();
		Connection::e_result onWritable();
		Connection::e_result onReadable();
		int					 register_read_pipe(int epoll_fd);
		int					 register_write_pipe(int epoll_fd);
		std::string			 body_buffer() const;
		std::string			 raw_output() const;

		pid_t		_pid;
		int			_wstatus;
		int			_stdin_pipe[2]; // server -> CGI
		int			_stdout_pipe[2];// CGI -> server
		std::string _body_buffer;
		std::string _raw_output; // FIXME: should probably use std::vector<char> as by design std::string doesn't guarantee contiguous space

		Connection *_parentConnection;

	private:
		std::size_t bytes_sent, bytes_received;
	};

private:
	State _state;


	HttpResponse &_res;
	HttpRequest	 &_req;
	std::string	  _script_path;

	void _build_env(std::vector<std::string> &env);
	void _parse_output_into_response(CGISession &sess);

public:
	CgiHandler(HttpRequest		 &req,
			   const Location	 &loc,
			   const std::string &script_path,
			   HttpResponse		 &res);

	HttpResponse &res() const;
	HttpRequest	 &req() const;
	ConnWorker	 *wrkr;

	StatusCode handle(HttpRequest &req, HttpResponse &res);
};

#endif//CGIHANDLER_HPP
