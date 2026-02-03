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
#include "TCPServer.hpp"
#include "webserv.hpp"
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

	log  << "http://" << FT_BLUE << FT_BOLD << inet_ntoa(sock.sin_addr);
	log << FT_WHITE << ":" << FT_YELLOW << ntohs(sock.sin_port) << FT_RESET << " ";
}

static void log_status(std::stringstream &log, LogCategory cat)
{
	log << '[' << FT_BOLD;
	switch (cat)
	{
		case (ERROR):
			log << FT_RED << "ERROR";
			break;
		case (WARNING):
			log << FT_YELLOW << "WARNING";
			break;
		case (NOTICE):
			log << FT_MAGENTA << "NOTICE";
			break;
		case (CONNECTION):
			log << FT_GREEN << "CONNECT";
			break;
	}
	log << FT_RESET << ']';
}

void	log_connection(ConnWorker const& wrkr, ConnectStatus status)
{
	std::stringstream	log;

	log_time(log);
	log_server(log, wrkr.getConn().getSrv());

	switch (status) {
		case (CONNECT):
			log_status(log, CONNECTION);
			log << ' ' << "FD " << wrkr.getConnFd() << " : Client connection established";
			break;
		case (DISCONNECT):
			log_status(log, CONNECTION);
			log << ' ' << "FD " << wrkr.getConnFd() << " : Client connection closed";
			break;
		case (HANGUP):
			log_status(log, WARNING);
			log << ' ' << "FD " << wrkr.getConnFd() << " : Client connection hung up";
			break;
		case (ERR):
			log_status(log, ERROR);
			log << ' ' << "FD " << wrkr.getConnFd() << " : Client connection encounter an error";
			break;
	}

	std::cout << log.str() << std::endl;
}
