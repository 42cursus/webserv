/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 02:09:33 by abelov            #+#    #+#             */
/*   Updated: 2025/11/26 02:09:34 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include "HttpResponse.hpp"

CgiHandler::CgiHandler(HttpRequest& req, const LocationConfig& loc, const std::string& script_path, HttpResponse& res) :
    _state(), _pid(0),
    _stdin_pipe(), _stdout_pipe(),
	 _res(res), _req(req)
{
    (void)loc;
    (void)script_path;
}

void CgiHandler::_build_env(std::vector<std::string>& env)
{
    env.push_back("REQUEST_METHOD=" + _req.method);
    env.push_back("SCRIPT_NAME=" + this->_req.path);
    env.push_back("CONTENT_TYPE=" + _req.headers["content-type"]);
    env.push_back("CONTENT_LENGTH=" + _req.headers["content-length"]);
}

std::string	CgiHandler::raw_output(void) const
{
	return (_raw_output);
}

int	CgiHandler::do_run(void)
{
	_raw_output = "blahblahblah\n";
	_res.headers["content-type"] = "text/plain";
	_res.headers["Hey"] = "ho";
	return (200);
}
