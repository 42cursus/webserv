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
#include <exception>
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

std::string colour_num(int num)
{
	std::string out;

	switch (num % 7) {
		case (0):
			out += FT_RED;
			break;
		case (1):
			out += FT_GREEN;
			break;
		case (2):
			out += FT_YELLOW;
			break;
		case (3):
			out += FT_BLUE;
			break;
		case (4):
			out += FT_MAGENTA;
			break;
		case (5):
			out += FT_CYAN;
			break;
		case (6):
			out += FT_WHITE;
			break;
	}

	return out;
}

static void log_server(std::stringstream &log, TCPServer const& srv)
{
	sockaddr_in const& sock = srv.getCfg().http.server.ipv4_listen;

	log  << "http://" << FT_BLUE << FT_BOLD << inet_ntoa(sock.sin_addr);
	log << FT_WHITE << ":" << colour_num(srv.idx * 2) << ntohs(sock.sin_port) << FT_RESET << " ";
}

static void log_status(std::stringstream &log, LogCategory cat)
{
	log << '[' << FT_BOLD;
	switch (cat)
	{
		case (LOG_ERROR):
			log << FT_RED << "ERROR" << FT_RESET "]  ";
			break;
		case (LOG_WARNING):
			log << FT_YELLOW << "WARNING" << FT_RESET "]";
			break;
		case (LOG_NOTICE):
			log << FT_MAGENTA << "NOTICE" << FT_RESET "] ";
			break;
		case (LOG_CONNECTION):
			log << FT_GREEN << "CONNECT" << FT_RESET "]";
			break;
		case (LOG_IO):
			log << FT_BLUE << "I/O" FT_RESET << "]    ";
			break;
		case (LOG_SERVER):
			log << FT_CYAN << "SERVER" FT_RESET << "] ";
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
		case (CONN_CONNECT):
			log_status(log, LOG_CONNECTION);
			log << ' ' << wrkr.getConn().getIpStr()
				<< " : Client connection " FT_GREEN "established" FT_RESET;
			break;
		case (CONN_DISCONNECT):
			log_status(log, LOG_CONNECTION);
			log << ' ' << wrkr.getConn().getIpStr()
				<< " : Client connection " FT_MAGENTA "disconnected" FT_RESET;
			break;
		case (CONN_HANGUP):
			log_status(log, LOG_WARNING);
			log << ' ' << wrkr.getConn().getIpStr()
				<< " : Client connection " FT_YELLOW "hung up" FT_RESET;
			break;
		case (CONN_ERROR):
			log_status(log, LOG_WARNING);
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
	log_status(log, LOG_IO);

	log << ' ' << conn.getIpStr() << " : ";
	log << FT_CYAN FT_BOLD << "REQUEST" FT_RESET  " \""
		<< req.method << " " << req.path << " " << req.protocol
		<< "\" ";
	log << "headers=" << req.header_len << " body=" << req.content_length;

	std::cout << log.str() << std::endl;
}

void	log_request_error(Connection const& conn, std::exception &e)
{
	std::stringstream	log;

	log_time(log);
	log_server(log, conn.getSrv());
	log_status(log, LOG_WARNING);

	log << ' ' << conn.getIpStr() << " : ";
	log << e.what();

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
	log_status(log, LOG_IO);

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

void log_title(void)
{
	std::string title =	FT_BLUE " _       __     __                        \n"
						FT_BLUE "| |     / /__  / /_  ________  ______   __ "    FT_RED "  ___   _______ \n"
						FT_BLUE "| | /| / / _ \\/ __ \\/ ___/ _ \\/ ___/ | / / " FT_RED " / _ \\ / __/ _ \\\n"
						FT_BLUE "| |/ |/ /  __/ /_/ (__  )  __/ /   | |/ /  "    FT_RED "/ // // _ \\\\_, /\n"
						FT_BLUE "|__/|__/\\___/_.___/____/\\___/_/    |___/   "  FT_RED "\\___(_)___/___/ \n";

	std::cout << FT_BOLD << title << FT_RESET << std::endl;
}

void	log_parsing(std::string& filename)
{
	std::stringstream	log;

	log_time(log);
	log_status(log, LOG_SERVER);
	log << " Parsing config file : " << FT_BOLD << FT_MAGENTA << filename << FT_RESET;

	std::cout << log.str() << std::endl;
}

void	log_parsing_error(std::exception &e)
{
	std::stringstream	log;

	log_time(log);
	log_status(log, LOG_ERROR);
	log << ' ' << e.what();

	std::cout << log.str() << std::endl;
}

void log_startup(TCPServer const& srv)
{
	std::stringstream	log;

	log_time(log);
	log_status(log, LOG_SERVER);
	log << " Server started at : ";
	log_server(log, srv);

	std::cout << log.str() << std::endl;
}
