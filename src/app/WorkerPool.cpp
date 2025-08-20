/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WorkerPool.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsmyth <fsmyth@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 20:00:30 by fsmyth            #+#    #+#             */
/*   Updated: 2025/08/20 21:08:34 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WorkerPool.hpp"
#include "TCPServer.hpp"
#include "Worker.hpp"

WorkerPool::WorkerPool(TCPServer& srv, size_t size) : _srv(srv), _allocp(0)
{
	_size = size;
	_pool.reserve(_size);
	for (size_t i = 0; i < _size; i++)
		_pool.push_back(Worker(_srv));
}

WorkerPool::~WorkerPool(void)
{
	std::cout << "WorkerPool {" << std::endl;
	std::cout << "\tsize: " << _size << std::endl;
	std::cout << "\tfreeList len: " << _freeList.size() << std::endl;
	std::cout << "\tallocp: " << _allocp << std::endl;
	std::cout << '}' << std::endl;
}

Worker*	WorkerPool::alloc(void)
{
	Worker* out;

	if (_freeList.empty())
	{
		if (_allocp == _size)
		{
			_pool.reserve(_size * 2);
			for (size_t i = _allocp; i < _size; i++)
				_pool.push_back(Worker(_srv));
			_size *= 2;
		}
		out = &_pool[_allocp++];
	}
	else
	{
		out = _freeList.front();
		_freeList.pop_front();
	}
	out->clearRequest();
	return (out);
}

void	WorkerPool::free(Worker* wrkr)
{
	if (wrkr == &_pool[_allocp - 1])
		_allocp--;
	else
		_freeList.push_front(wrkr);
}
