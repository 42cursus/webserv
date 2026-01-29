/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WorkerPool.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsmyth <fsmyth@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/20 20:00:33 by fsmyth            #+#    #+#             */
/*   Updated: 2025/08/20 20:29:00 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef WORKERPOOL_HPP
# define WORKERPOOL_HPP

#include "ConnWorker.hpp"
#include "TCPServer.hpp"
#include <list>
#include <vector>

class WorkerPool
{

private:
	std::list<std::vector<ConnWorker> >	_pool;
	std::list<ConnWorker *>				_freeList;
	size_t							_allocp;
	size_t							_size;
	size_t							_nodesize;
	size_t							_num_alloced;

    ConnWorker *		_getWorker(size_t index);

public:
	WorkerPool(size_t size);
	~WorkerPool();

    ConnWorker *	alloc(TCPServer *srv);
	void	free(ConnWorker *wrkr);
	size_t	getNumAlloced(void) const;
};

#endif
