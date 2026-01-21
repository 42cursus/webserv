/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:02:20 by abelov            #+#    #+#             */
/*   Updated: 2026/01/21 18:16:04 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"
#include "src/http/HttpRequest.hpp"
#include "src/http/HttpResponse.hpp"
#include <cctype>
#include <cstring>
#include <iomanip>
#include <sys/types.h>
#include <cstdlib>

Worker::Worker(TCPServer &srv)
	: _req_buffer(),
	_req_status(REQ_BODY),
	_socket_fd(),
	_addr(),
	_addr_size(),
	srv(srv)
{

}

Worker::~Worker()
{

}

void Worker::acceptConnection()
{
	_socket_fd = accept(srv.getSocketFd(), (struct sockaddr*)&_addr, &_addr_size);
	if (_socket_fd < 0) {
		std::cerr << "Failed to accept client request." << std::endl;
		throw Worker::GenericException();
	}
	std::cout << "Accepted connection. fd: " << _socket_fd << std::endl;
}

void logServingFile(const std::string& path, const std::string& mimetype) {
	std::cout << "Serving file: " << path << " with MIME type: " << mimetype << std::endl;
}

size_t	Worker::extract_body(size_t nread, size_t old_size, size_t clcr_pos)
{
	size_t	body_start = clcr_pos + 4 - old_size;
	size_t	body_size = nread - body_start;

	_req->body.resize(body_size);
	std::memcpy(_req->body.data(), &_req_buffer[body_start], body_size);
	return (body_size);
}

int Worker::handleRequest()
{
	int			nread;

	nread = read(_socket_fd, _req_buffer, 1023);
	if (nread <= 0)
		return (2);
	_req_buffer[nread] = '\0';
	switch (_req_status) {
		case (REQ_HEADERS): {
			size_t	old_size = _rawRequest.size();
			_rawRequest += _req_buffer;

			size_t	clcr_pos = _rawRequest.find("\r\n\r\n");
			if (clcr_pos == _rawRequest.npos)
				return (1);
			std::cout << "\e[35m" << "Request ready on fd: " << _socket_fd << std::endl;
			std::cout << "\e[32m" << _rawRequest.substr(0, clcr_pos + 4) << "\e[31m" << std::endl;
			// for (int i = 0; _rawRequest[i] != 0 && i < 1024; i++)
			// {
			// 	std::cout << (int)_rawRequest[i] << ' ';
			// 	if (_rawRequest[i] == '\n')
			// 		std::cout << std::endl;
			// }
			// std::cout << "\e[m" << std::endl;
			_req = new HttpRequest();
			try {
				_req->parseRequest(_rawRequest);
			}
			catch (HttpRequest::GenericException &e) {
				std::cout << e.what() << std::endl;
				_rawRequest.erase();
				delete _req;
				return (0);
			}
			_req->content_length = std::atoi(_req->headers["content-length"].c_str());
			if (_req->content_length > 0)
			{
				_req_status = REQ_BODY;
				size_t	body_size = extract_body(nread, old_size, clcr_pos);
				if (body_size < _req->content_length)
					return (1);
				size_t i;
				for (i = 0; i < _req->body.size() && i < 1000; i++)
				{
					char c = _req->body[i];
					if (std::isprint(c))
						// std::cout << "\e[32m" << std::setw(2) << c << ' ';
						std::cout << "\e[32m" << c;
					else
						std::cout << "\e[31m " << std::hex << std::setw(2) << std::setfill(' ') << std::setfill('0') << (int)(u_char)c << ' ';
				}
				if (i < _req->body.size())
					std::cout << "\e[34;1m [...]";
				std::cout << "\e[m" << std::endl;
				}
			break ;
		}
		case (REQ_BODY): {
			size_t old_size = _req->body.size();
			_req->body.resize(old_size + nread);
			std::memcpy(_req->body.data() + old_size, _req_buffer, nread);
			if (_req->body.size() < _req->content_length)
				return (1);
			size_t i;
			for (i = 0; i < _req->body.size() && i < 1000; i++)
			{
				char c = _req->body[i];
				if (std::isprint(c))
					// std::cout << "\e[32m" << std::setw(2) << std::setfill(' ') << c << ' ';
					std::cout << "\e[32m" << c;
				else
					std::cout << "\e[31m " << std::hex << std::setw(2) << std::setfill('0') << (int)(u_char)c << ' ';
			}
			if (i < _req->body.size())
				std::cout << "\e[34;1m [...]";
			std::cout << "\e[m" << std::endl;
			break ;
		}
		default:
			break ;
	}

	HttpResponse res = HttpResponse();
	res.statuscode = "200";
	res.statusmsg = "OK";
	res.headers = _req->headers;
	

	std::map<const std::string, std::string>::iterator it = _req->headers.begin();
	while (it != _req->headers.end())
	{
		std::cout << it->first << " : " << it->second << std::endl;
		it++;
	}
	std::string mimetype = _req->getMimeType(_req->path);
	res.body = _req->getHtmlResponse(srv.getCfg());

	std::string response = res.buildHttpResponse(res.statuscode, res.statusmsg, res.headers, res.body, mimetype);
	logServingFile(_req->path, mimetype);

	write(_socket_fd, response.c_str(), response.length());
	// close(_socket_fd);
	_rawRequest.erase();
	delete _req;
	return (0);
}

const char *Worker::GenericException::what() const throw()
{
	return "Client exception happened";
}

// Worker&	Worker::operator=(Worker const &src)
// {
// 	this->_socket_fd = src._socket_fd;
// 	this->srv = src.srv;
// 	return (*this);
// }

int Worker::getSocketFd() const
{
	return _socket_fd;
}

int Worker::requestHandled() const
{
	return _request_handled;
}

std::string& Worker::getRawRequest()
{
	return _rawRequest;
}

void	Worker::clearRequest(void)
{
	if (!_rawRequest.empty())
		_rawRequest.erase();
	_req_status = REQ_HEADERS;
}
