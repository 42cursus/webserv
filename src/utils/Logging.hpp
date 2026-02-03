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

class HttpRequest;
class HttpResponse;
class ConnWorker;

enum LogCategory
{
	ERROR,
	WARNING,
	NOTICE,
	CONNECTION,
};

enum ConnectStatus
{
	CONNECT,
	DISCONNECT,
	HANGUP,
	ERR,
};

void	log_connection(ConnWorker const& wrkr, ConnectStatus status);

#endif
