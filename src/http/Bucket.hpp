/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bucket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 18:13:46 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 18:13:46 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUCKET_HPP
#define BUCKET_HPP

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

class Bucket {
public:
	enum e_type {
		MEMORY = 0,
		FILE_SEGMENT,
		PIPE_SEGMENT,
		EOS
	};

	Bucket();

	static Bucket memory(const char *data, size_t len);
	static Bucket memory(const std::string &data);
	static Bucket eos();

	e_type getType() const;
	size_t readableBytes() const;
	bool   empty() const;
	void   consume(size_t nbytes);
	std::string viewAsString() const;

private:
	e_type			   _type;
	std::vector<char> _data;
	size_t			   _offset;
};

#endif//BUCKET_HPP
