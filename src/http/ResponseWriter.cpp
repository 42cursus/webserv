/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseWriter.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:01:00 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 16:01:00 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ResponseWriter.hpp"
#include "Connection.hpp"
#include "Logging.hpp"
#include <algorithm>
#include <cerrno>
#include <unistd.h>

ResponseWriter::e_result ResponseWriter::writeCurrent(Connection const &conn, int fd, HttpResponse &res)
{
	std::string &response = res.response;
	if (res.start >= response.size()) {
		if (res.chunk_start < res.body.size() || res.body_complete) {
			res.start = 0;
			res.response = res.chunk_response(32768);
			if (res.body_complete)
				res.chunking_express = true;
			log_chunk_response(conn, res);
		} else if (!res.chunked) {
			return WR_RESP_COMPLETE;
		} else {
			return WR_WANT_WRITE;
		}
	}

	if (res.start == 0 && res.chunk_start == 0)
		log_response(conn, res);

	size_t remaining = response.length() - res.start;
	remaining = std::min(remaining, response.length() - res.start);
	ssize_t bytesWritten = ::write(fd, response.data() + res.start, remaining);

	if (bytesWritten > 0) {
		res.start += static_cast<size_t>(bytesWritten);
		if (res.start >= response.size()) {
			if (res.chunked && !res.chunking_express) {
				if (res.chunk_start >= res.body.size()) {
					if (res.body_complete)
						res.chunking_express = true;
					else
						return WR_WANT_WRITE;
				}
				res.start = 0;
				res.response = res.chunk_response(32768);
				log_chunk_response(conn, res);
				return WR_WANT_WRITE;
			}
			return WR_RESP_COMPLETE;
		}
		return WR_WANT_WRITE;
	}

	if (bytesWritten == 0)
		return WR_CLOSED;

	if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
		return WR_WANT_WRITE;

	if (errno == EPIPE || errno == ECONNRESET)
		return WR_CLOSED;

	return WR_ERROR;
}
