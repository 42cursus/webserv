/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WorkerPool.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsmyth <fsmyth@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 20:00:30 by fsmyth            #+#    #+#             */
/*   Updated: 2026/01/22 14:55:28 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WorkerPool.hpp"
#include "TCPServer.hpp"
#include "Worker.hpp"
#include <cstddef>
#include <vector>

WorkerPool::WorkerPool(size_t size) : _allocp(0)
{
	_size = size;
	_num_alloced = 0;
	_nodesize = size;
	_pool.push_front(std::vector<Worker>());
	_pool.front().reserve(_nodesize);
	for (size_t i = 0; i < _nodesize; i++)
		_pool.front().push_back(Worker());
}

WorkerPool::~WorkerPool(void)
{
	Worker*	wrkr;

	for (size_t i = 0; i < _size; i++)
	{
		wrkr = _getWorker(i);
		if (wrkr->getConnFd() != -1)
		{
			std::cout << "Pruning orphaned worker with fd " << wrkr->getConnFd() << std::endl;
			wrkr->closeSocketFd();
		}
	}
}

Worker*	WorkerPool::alloc(TCPServer *srv)
{
	Worker* out;

	if (_freeList.empty())
	{
		if (_allocp == _size)
		{
			// std::cout << "growing pool: " << _size << " to " << _size + _nodesize << std::endl;
			_size += _nodesize;
			_pool.push_back(std::vector<Worker>());
			_pool.back().reserve(_nodesize);
			for (size_t i = 0; i < _nodesize; i++)
				_pool.back().push_back(Worker());
		}
		out = _getWorker(_allocp);
		// std::cout << "Getting worker: " << out << " allocp: " << _allocp << std::endl;
		_allocp++;
	}
	else
	{
		out = _freeList.front();
		_freeList.pop_front();
	}
	_num_alloced++;
	out->setSrv(srv);
	out->clearRequest();
	return (out);
}

Worker*	WorkerPool::_getWorker(size_t index)
{
	std::list<std::vector<Worker> >::iterator	it = _pool.begin();

	for (size_t total = _nodesize - 1; total < index; total += _nodesize)
		it++;

	return (&it->data()[index % _nodesize]);
}


void	WorkerPool::free(Worker* wrkr)
{
	if (wrkr == _getWorker(_allocp - 1))
		_allocp--;
	else
		_freeList.push_front(wrkr);
	wrkr->closeSocketFd();
	_num_alloced--;
}

size_t	WorkerPool::getNumAlloced(void) const
{
	return (_num_alloced);
}
