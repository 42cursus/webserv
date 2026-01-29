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

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <string>
#include <vector>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Location.hpp"
#include "IHandler.hpp"

class CgiHandler  : public IHandler {
public:
    enum State {
        READY,
        WRITING_BODY,
        READING_OUTPUT,
        DONE,
        ERROR
    };

    CgiHandler(const Location &loc);

    std::string body_buffer() const;

    std::string raw_output() const;

    HttpResponse& res() const;
    HttpRequest& req() const;

    int handle(HttpRequest& req, HttpResponse& res);

private:
    State               _state;
    pid_t               _pid;
    int                 _stdin_pipe[2];   // server -> CGI
    int                 _stdout_pipe[2];  // CGI -> server
    std::string         _body_buffer;
    std::string         _raw_output;
//    const std::string   &_script_path;
    const Location& _loc;
    void _build_env(std::vector<std::string> &env);
    void _parse_output_into_response();
};

#endif //CGIHANDLER_HPP
