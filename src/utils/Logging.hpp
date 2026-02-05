/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logging.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsmyth <fsmyth@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 23:24:22 by fsmyth            #+#    #+#             */
/*   Updated: 2026/02/03 00:55:50 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGING_HPP
# define LOGGING_HPP

#include <string>

class HttpRequest;
class HttpResponse;
class ConnWorker;
class Connection;
class TCPServer;

enum LogCategory
{
	LOG_ERROR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_CONNECTION,
	LOG_IO,
	LOG_SERVER,
};

enum ConnectStatus
{
	CONN_CONNECT,
	CONN_DISCONNECT,
	CONN_HANGUP,
	CONN_ERROR,
};

enum StartupCategory
{
	SU_SOCKET_CREATE,
	SU_SOCKET_BIND,
	SU_SOCKET_LISTEN,
	SU_SERVER_STARTED,
};

void	log_title(void);

void	log_parsing(std::string& filename);
void	log_parsing_error(std::exception &e);

void	log_startup(TCPServer const& srv, StartupCategory type);
void	log_startup_error(TCPServer const& srv, StartupCategory type);
void	log_no_servers(void);
void	log_server_stop(TCPServer const& srv);
void	log_shutdown(void);
void	log_prune(ConnWorker const& wrkr);

void	log_connection(ConnWorker const& wrkr, ConnectStatus status);

void	log_request(Connection const& conn, HttpRequest const& req);
void	log_request_error(Connection const& conn, std::exception &e);
void	log_response(Connection const& conn, HttpResponse const& res);

std::string	colour_num(int num);

#endif
