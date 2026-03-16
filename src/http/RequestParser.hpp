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

#include "HttpRequest.hpp"
#include <cstddef>
#include <string>

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
};

#endif//REQUESTPARSER_HPP
