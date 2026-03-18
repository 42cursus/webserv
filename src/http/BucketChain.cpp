/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BucketChain.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 18:42:29 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 18:42:29 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "BucketChain.hpp"

/*
** -------------------------------- STATIC VARS -------------------------------
*/

/*
** ------------------------------- CONSTRUCTORS -------------------------------
*/

BucketChain::BucketChain() : _buckets(), _bytes(0)
{}

/*
** ------------------------------- DESTRUCTORS --------------------------------
*/


/*
** -------------------------------- OPERATORS ---------------------------------
*/

/*
** -------------------------------- OVERLOADS ---------------------------------
*/

/*
** --------------------------------- METHODS ----------------------------------
*/

bool BucketChain::empty() const
{
	return _buckets.empty();
}

size_t BucketChain::bytes() const
{
	return _bytes;
}

size_t BucketChain::bucketCount() const
{
	return _buckets.size();
}

bool BucketChain::hasEOS() const
{
	for (std::deque<Bucket>::const_iterator it = _buckets.begin(); it != _buckets.end(); ++it) {
		if (it->getType() == Bucket::EOS)
			return true;
	}
	return false;
}

void BucketChain::append(const Bucket &bucket)
{
	_buckets.push_back(bucket);
	_bytes += bucket.readableBytes();
}

void BucketChain::appendMemory(const char *data, size_t len)
{
	if (len == 0)
		return;
	append(Bucket::memory(data, len));
}

void BucketChain::appendString(const std::string &data)
{
	appendMemory(data.data(), data.size());
}

void BucketChain::appendEOS()
{
	append(Bucket::eos());
}

void BucketChain::consume(size_t nbytes)
{
	size_t remaining = nbytes;
	while (remaining > 0 && !_buckets.empty()) {
		Bucket &front = _buckets.front();
		size_t before = front.readableBytes();
		front.consume(remaining);
		size_t consumed = before - front.readableBytes();
		remaining -= consumed;
		_bytes -= consumed;
		if (front.empty() || front.getType() == Bucket::EOS)
			_buckets.pop_front();
	}
}

void BucketChain::clear()
{
	_buckets.clear();
	_bytes = 0;
}

std::string BucketChain::flatten(size_t max_bytes) const
{
	std::string out;
	size_t limit;

	if (max_bytes == 0)
		limit = _bytes;
	else
		limit = std::min(max_bytes, _bytes);

	out.reserve(limit);
	size_t produced = 0;
	std::deque<Bucket>::const_iterator it = _buckets.begin();
	for (; it != _buckets.end() && produced < limit; ++it) {
		const size_t n = std::min(it->readableBytes(), limit - produced);
		const char *p = it->dataPtr();
		if (p == NULL || n == 0)
			continue;
		out.append(p, n);
		produced += n;
	}
	return out;
}


/*
** -------------------------------- ACCESSORS ---------------------------------
*/

/*
** -------------------------------- EXCEPTIONS --------------------------------
*/

/*
** -------------------------------- MISCELLANEOUS --------------------------------
*/



