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

#include <vector>
#include <list>
#include "Worker.hpp"
#include "TCPServer.hpp"

class WorkerPool
{

private:
	std::list<std::vector<Worker> >				_pool;
	std::list<Worker *>							_freeList;
	TCPServer&									_srv;
	size_t										_allocp;
	size_t										_size;
	size_t										_nodesize;

	Worker*		_getWorker(size_t index);

public:
	WorkerPool(TCPServer& srv, size_t size = 1024);
	~WorkerPool();

	Worker*	alloc(void);
	void	free(Worker *wrkr);
};

#endif
