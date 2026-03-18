/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BucketChain.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 18:42:18 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 18:42:18 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUCKETCHAIN_HPP
#define BUCKETCHAIN_HPP

#include "Bucket.hpp"
#include <cstddef>
#include <deque>
#include <string>

class Bucket;

class BucketChain {
public:
	BucketChain();

	bool   empty() const;
	std::size_t bytes() const;
	std::size_t bucketCount() const;
	bool   hasEOS() const;

	void append(const Bucket &bucket);
	void appendMemory(const char *data, size_t len);
	void appendString(const std::string &data);
	void appendEOS();
	void consume(size_t nbytes);
	void clear();
	std::string flatten(size_t max_bytes) const;

private:
	std::deque<Bucket> _buckets;
	size_t			   _bytes;
};

#endif //BUCKETCHAIN_HPP
