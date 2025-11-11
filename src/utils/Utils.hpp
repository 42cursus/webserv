/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/25 23:55:51 by abelov            #+#    #+#             */
/*   Updated: 2025/10/25 23:55:51 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <vector>

class Utils
{
public:
	template <typename T, size_t N>
	static std::vector<T> to_vector(const T (&arr)[N])
	{
		std::vector<T> v;
		v.reserve(N);

		size_t i = 0;
		while (i < N)
			v.push_back(arr[i++]);
		return v;
	}

	template <typename T, size_t N>
	static size_t countof(const T (& /*unused*/)[N]) { return N; }
};

#endif //UTILS_HPP
