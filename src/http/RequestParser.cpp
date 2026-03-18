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
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace {
bool parse_hex_size(const std::string &size_line, size_t &chunk_size)
{
	std::string token = size_line;
	size_t semicolon = token.find(';');
	if (semicolon != std::string::npos)
		token = token.substr(0, semicolon);
	if (token.empty())
		return false;
	std::istringstream iss(token);
	iss >> std::hex >> chunk_size;
	return !(iss.fail() || !iss.eof());
}

RequestParser::Result parse_chunked_body(const std::string &buffer, size_t in_offset, size_t header_bytes, HttpRequest *req)
{
	RequestParser::Result res;
	std::vector<char> decoded;
	size_t pos = in_offset + header_bytes;

	while (true) {
		size_t line_end = buffer.find(CRLF, pos);
		if (line_end == std::string::npos) {
			delete req;
			return res;
		}

		size_t chunk_size = 0;
		if (!parse_hex_size(buffer.substr(pos, line_end - pos), chunk_size)) {
			res.status = RequestParser::BAD_REQUEST;
			res.error_message = "Malformed chunk size";
			delete req;
			return res;
		}
		pos = line_end + 2;

		if (chunk_size == 0) {
			if (buffer.size() < pos + 2) {
				delete req;
				return res;
			}
			if (buffer.compare(pos, 2, CRLF) == 0) {
				pos += 2;
			} else {
				size_t trailers_end = buffer.find(CRLF CRLF, pos);
				if (trailers_end == std::string::npos) {
					delete req;
					return res;
				}
				pos = trailers_end + 4;
			}
			break;
		}

		if (buffer.size() < pos + chunk_size + 2) {
			delete req;
			return res;
		}
		decoded.insert(decoded.end(), buffer.begin() + static_cast<long>(pos), buffer.begin() + static_cast<long>(pos + chunk_size));
		pos += chunk_size;
		if (buffer.compare(pos, 2, CRLF) != 0) {
			res.status = RequestParser::BAD_REQUEST;
			res.error_message = "Malformed chunk body";
			delete req;
			return res;
		}
		pos += 2;
	}

	req->body = decoded;
	req->content_length = decoded.size();
	res.status = RequestParser::REQUEST_READY;
	res.request = req;
	res.consumed_bytes = pos - in_offset;
	return res;
}
}

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
	bool is_chunked = false;
	if (req->headers.count("transfer-encoding")) {
		std::string te = req->headers["transfer-encoding"];
		for (size_t i = 0; i < te.size(); ++i)
			te[i] = static_cast<char>(std::tolower(te[i]));
		if (te.find("chunked") != std::string::npos)
			is_chunked = true;
	}
	if (is_chunked)
		return parse_chunked_body(buffer, in_offset, header_bytes, req);

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

RequestParser::Result RequestParser::tryExtract(const BucketChain &buffer_chain)
{
	return tryExtract(buffer_chain.flatten(0), 0);
}
