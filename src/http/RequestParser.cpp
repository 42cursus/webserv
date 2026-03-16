/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 15:18:53 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 15:18:53 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestParser.hpp"
#include <cstdlib>
#include <cstring>

RequestParser::Result::Result() :
	status(NEED_MORE_DATA),
	request(NULL),
	consumed_bytes(0),
	error_message()
{}

RequestParser::Result RequestParser::tryExtract(const std::string &buffer, size_t in_offset)
{
	RequestParser::Result res;
	size_t				  hdr_end = buffer.find(CRLF CRLF, in_offset);

	if (hdr_end == std::string::npos) {
		res.status = NEED_MORE_DATA;
		return res;
	}

	const size_t	  header_bytes = (hdr_end - in_offset) + 4;
	const std::string header_block = buffer.substr(in_offset, header_bytes);
	HttpRequest *	  req = new HttpRequest();
	try {
		req->parseRequest(header_block);
	} catch (std::exception &e) {
		res.status = BAD_REQUEST;
		res.error_message = e.what();
		delete req;
		return res;
	}

	size_t content_length = 0;
	if (req->headers.count("content-length"))
		content_length = static_cast<size_t>(std::atoi(req->headers["content-length"].c_str()));
	req->content_length = content_length;

	const size_t need_total = header_bytes + content_length;
	if (buffer.size() - in_offset < need_total) {
		delete req;
		return res;
	}

	if (content_length > 0) {
		req->body.resize(content_length);
		std::memcpy(&req->body[0], &buffer[0] + in_offset + header_bytes, content_length);
	}

	res.status = REQUEST_READY;
	res.request = req;
	res.consumed_bytes = need_total;
	return res;
}
