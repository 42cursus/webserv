/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logging.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsmyth <fsmyth@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 23:24:15 by fsmyth            #+#    #+#             */
/*   Updated: 2026/02/03 00:56:58 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logging.hpp"
#include "ConnWorker.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "StatusCode.hpp"
#include "TCPServer.hpp"
#include "webserv.hpp"
#include <iomanip>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sstream>

static void log_time(std::stringstream &log)
{
	time_t	now = time(NULL);
	tm		*ts = localtime(&now);
	char	buf[64];

	strftime(buf, 64, "[%Y-%m-%d %H:%M:%S] ", ts);
	log << buf;
}

static void log_server(std::stringstream &log, TCPServer const& srv)
{
	sockaddr_in const& sock = srv.getCfg().http.server.ipv4_listen;
	// struct addrinfo hints;
	// struct addrinfo *res;
	//
	// hints.ai_family = AF_UNSPEC;
	// hints.ai_flags = AI_CANONNAME;
	// hints.ai_socktype = SOCK_STREAM;
	// getaddrinfo("localhost", "8080", &hints, &res);
	//
	// for (; res != NULL; res = res->ai_next)
	// {
	//   std::cout << res->ai_canonname << std::endl;
	// }
	

	log  << "http://" << FT_BLUE << FT_BOLD << inet_ntoa(sock.sin_addr);
	log << FT_WHITE << ":" << FT_YELLOW << ntohs(sock.sin_port) << FT_RESET << " ";
}

static void log_status(std::stringstream &log, LogCategory cat)
{
	log << '[' << FT_BOLD;
	switch (cat)
	{
		case (ERROR):
			log << FT_RED << "ERROR" << FT_RESET "]  ";
			break;
		case (WARNING):
			log << FT_YELLOW << "WARNING" << FT_RESET "]";
			break;
		case (NOTICE):
			log << FT_MAGENTA << "NOTICE" << FT_RESET "] ";
			break;
		case (CONNECTION):
			log << FT_GREEN << "CONNECT" << FT_RESET "]";
			break;
		case (IO):
			log << FT_BLUE << "I/O" FT_RESET << "]    ";
			break;
	}
}

void	log_connection(ConnWorker const& wrkr, ConnectStatus status)
{
	std::stringstream	log;
	// struct sockaddr_in	_addr;
	// socklen_t			_addr_size = sizeof(_addr);
	// struct sockaddr		*addr = reinterpret_cast<struct sockaddr*>(&_addr);


	// getsockname(wrkr.getConnFd(), addr, &_addr_size);
	// std::cout << inet_ntoa(_addr.sin_addr) << ":" << ntohs(_addr.sin_port) << std::endl;
	log_time(log);
	log_server(log, wrkr.getConn().getSrv());

	switch (status) {
		case (CONNECT):
			log_status(log, CONNECTION);
			log << ' ' << wrkr.getConn().getIpStr()
				<< " : Client connection " FT_GREEN "established" FT_RESET;
			break;
		case (DISCONNECT):
			log_status(log, CONNECTION);
			log << ' ' << wrkr.getConn().getIpStr()
				<< " : Client connection " FT_MAGENTA "disconnected" FT_RESET;
			break;
		case (HANGUP):
			log_status(log, WARNING);
			log << ' ' << wrkr.getConn().getIpStr()
				<< " : Client connection " FT_YELLOW "hung up" FT_RESET;
			break;
		case (ERR):
			log_status(log, ERROR);
			log << ' ' << wrkr.getConn().getIpStr()
				<< " : Client connection " FT_RED "encountered an error" FT_RESET;
			break;
	}

	std::cout << log.str() << std::endl;
}

void log_request(Connection const& conn, HttpRequest const& req)
{
	std::stringstream	log;

	log_time(log);
	log_server(log, conn.getSrv());
	log_status(log, IO);

	log << ' ' << conn.getIpStr() << " : ";
	log << FT_CYAN FT_BOLD << "REQUEST" FT_RESET  " \""
		<< req.method << " " << req.path << " " << req.protocol
		<< "\" ";
	log << "headers=" << req.header_len << " body=" << req.content_length;

	std::cout << log.str() << std::endl;
}

static void log_statuscode(std::stringstream &log, std::string code)
{
	int type = std::atoi(code.c_str()) / 100;

	log << FT_BOLD;
	switch (type) {
		case (1):
			log << FT_MAGENTA;
			break;
		case (2):
			log << FT_GREEN;
			break;
		case (3):
			log << FT_BLUE;
			break;
		case (4):
			log << FT_YELLOW;
			break;
		case (5):
			log << FT_RED;
			break;
		default:
			break;
	}
	log << code;
	log << FT_RESET;
}

void log_response(Connection const& conn, HttpResponse const& res)
{
	std::stringstream	log;

	log_time(log);
	log_server(log, conn.getSrv());
	log_status(log, IO);

	log << ' ' << conn.getIpStr() << " : ";
	log << FT_MAGENTA FT_BOLD << "RESPONSE " FT_RESET;
	log_statuscode(log, res.statuscode);
	log << " sent=" << res.response.length()
		<< " body=" << res.headers.find("content-length")->second;

	std::map<std::string, std::string>::const_iterator it;
	if ((it = res.headers.find("content-type")) != res.headers.end())
		log << " type=" << it->second;

	std::cout << log.str() << std::endl;
}
