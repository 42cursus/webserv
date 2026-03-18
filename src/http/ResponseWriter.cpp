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



ResponseWriter::e_result ResponseWriter::writeCurrent(Connection &conn, int fd, HttpResponse &res)
{
	static const size_t kWriteChunkBytes = DEFAULT_CHUNK_SIZE;
	std::string &response = res.response;
	BucketChain &out = conn.transportOutputBuckets();
	if (res.start >= response.size()) {
		out.clear();
		conn.refreshBackpressureState();
		if (res.chunk_start < res.body.size() || res.body_complete) {
			res.start = 0;
			res.response = res.chunk_response(DEFAULT_CHUNK_SIZE);
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

	if (out.bytes() == 0 && res.start < response.size()) {
		const size_t remaining = response.size() - res.start;
		const size_t staged = std::min(remaining, kWriteChunkBytes);
		out.appendMemory(response.data() + res.start, staged);
		conn.refreshBackpressureState();
	}

	std::string wire = out.flatten(kWriteChunkBytes);
	if (wire.empty())
		return WR_WANT_WRITE;

	ssize_t bytesWritten = ::write(fd, wire.data(), wire.size());

	if (bytesWritten > 0) {
		const size_t consumed = static_cast<size_t>(bytesWritten);
		out.consume(consumed);
		res.start += consumed;
		conn.refreshBackpressureState();
		if (res.start >= response.size()) {
			if (res.chunked && !res.chunking_express) {
				if (res.chunk_start >= res.body.size()) {
					if (res.body_complete)
						res.chunking_express = true;
					else
						return WR_WANT_WRITE;
				}
				res.start = 0;
				res.response = res.chunk_response(DEFAULT_CHUNK_SIZE);
				out.clear();
				conn.refreshBackpressureState();
				log_chunk_response(conn, res);
				return WR_WANT_WRITE;
			}
			out.clear();
			conn.refreshBackpressureState();
			return WR_RESP_COMPLETE;
		}
		return WR_WANT_WRITE;
	}

	if (bytesWritten == 0)
		return WR_CLOSED;

	if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) // FIXME: CAN'T DO THAT!!!
		return WR_WANT_WRITE;

	if (errno == EPIPE || errno == ECONNRESET) // FIXME: CAN'T DO THAT!!!
		return WR_CLOSED;

	return WR_ERROR;
}
