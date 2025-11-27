/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 01:47:47 by abelov            #+#    #+#             */
/*   Updated: 2025/11/26 01:47:47 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_CGIHANDLER_HPP
#define WEBSERV_CGIHANDLER_HPP

#include <string>
#include <vector>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationConfig.hpp"

class CgiHandler
{
public:
    enum State {
        READY,
        WRITING_BODY,
        READING_OUTPUT,
        DONE,
        ERROR
    };

    CgiHandler(HttpRequest &req,
               const LocationConfig &loc,
               const std::string &script_path,
			   HttpResponse& res);

    std::string body_buffer() const
    {
        return _body_buffer;
    }

    std::string raw_output() const;

    HttpResponse& res() const
    {
        return _res;
    }

    HttpRequest& req() const
    {
        return _req;
    }

	int	do_run(void);

private:
    State           _state;
    pid_t           _pid;
    int             _stdin_pipe[2];   // server -> CGI
    int             _stdout_pipe[2];  // CGI -> server
    std::string     _body_buffer;
    std::string     _raw_output;
    HttpResponse&    _res;
    HttpRequest&	_req;

    void _build_env(std::vector<std::string> &env);
    void _parse_output_into_response();
};

#endif //WEBSERV_CGIHANDLER_HPP
