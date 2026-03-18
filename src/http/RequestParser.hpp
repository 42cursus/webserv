/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 15:18:41 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 15:18:41 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include "BucketChain.hpp"
#include "HttpRequest.hpp"
#include <cstddef>
#include <string>

#ifndef CRLF
#define CRLF "\r\n"
#endif
#ifndef CHUNK_END
#define CHUNK_END "0\r\n\r\n"
#endif

class RequestParser {
public:
	enum e_status {
		NEED_MORE_DATA = 0,
		REQUEST_READY,
		BAD_REQUEST,
	};

	struct Result {
		e_status	status;
		HttpRequest	*request;
		size_t		consumed_bytes;
		std::string	error_message;

		Result();
	};

	static Result tryExtract(const std::string &buffer, size_t in_offset);
	static Result tryExtract(const BucketChain &buffer_chain);
};

#endif//REQUESTPARSER_HPP
