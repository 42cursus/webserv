/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseWriter.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 16:01:04 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 16:01:04 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef RESPONSEWRITER_HPP
#define RESPONSEWRITER_HPP

#include "HttpResponse.hpp"

#define DEFAULT_CHUNK_SIZE 32768
class Connection;

class ResponseWriter {
public:
	enum e_result {
		WR_OK = 0,
		WR_WANT_WRITE,
		WR_CLOSED,
		WR_ERROR,
		WR_RESP_COMPLETE,
	};

	static e_result writeCurrent(Connection &conn, int fd, HttpResponse &res);
};

#endif//RESPONSEWRITER_HPP
