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

CgiHandler::CgiHandler(const HttpRequest& req, const LocationConfig& loc, const std::string& script_path) :
    _state(), _pid(0),
    _stdin_pipe(), _stdout_pipe()
{
    (void)req;
    (void)loc;
    (void)script_path;
}

void CgiHandler::_build_env(std::vector<std::string>& env)
{
    env.push_back("REQUEST_METHOD=" + _req.method);
    env.push_back("SCRIPT_NAME=" + this->_req.path);
    env.push_back("CONTENT_TYPE=" + _req.headers["Content-Type"]);
    env.push_back("CONTENT_LENGTH=" + _req.headers["Content-Length"]);
}
