/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bucket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 18:13:40 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 18:13:40 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bucket.hpp"
#include <algorithm>

Bucket::Bucket() : _type(MEMORY), _data(), _offset(0)
{}

Bucket Bucket::memory(const char *data, size_t len)
{
	Bucket out;
	out._type = MEMORY;
	out._data.assign(data, data + len);
	out._offset = 0;
	return out;
}

Bucket Bucket::memory(const std::string &data)
{
	return memory(data.data(), data.size());
}

Bucket Bucket::eos()
{
	Bucket out;
	out._type = EOS;
	return out;
}

Bucket::e_type Bucket::getType() const
{
	return _type;
}

size_t Bucket::readableBytes() const
{
	if (_type == EOS || _offset >= _data.size())
		return 0;
	return _data.size() - _offset;
}

bool Bucket::empty() const
{
	return readableBytes() == 0;
}

void Bucket::consume(size_t nbytes)
{
	if (_type == EOS || nbytes == 0)
		return;
	const size_t n = std::min(nbytes, readableBytes());
	_offset += n;
}

const char *Bucket::dataPtr() const
{
	if (_type == EOS || _offset >= _data.size())
		return NULL;
	return &_data[_offset];
}

std::string Bucket::viewAsString() const
{
	if (_type == EOS)
		return "";
	return std::string(_data.begin() + _offset, _data.end());
}
