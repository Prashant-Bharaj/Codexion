/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   priority_queue.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prasingh <prasingh@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 00:00:00 by prasingh          #+#    #+#             */
/*   Updated: 2026/02/08 11:04:25 by prasingh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdlib.h>

void	*dongle_request_queue_create(int scheduler)
{
	t_priority_queue	*pq;

	pq = (t_priority_queue *)malloc(sizeof(t_priority_queue));
	if (!pq)
		return (NULL);
	pq->size = 0;
	pq->scheduler = scheduler;
	return (pq);
}

void	dongle_request_queue_destroy(void *queue)
{
	if (!queue)
		return ;
	free(queue);
}

void	dongle_request_queue_add(void *queue, int coder_id, long priority)
{
	t_priority_queue	*pq;

	if (!queue)
		return ;
	pq = (t_priority_queue *)queue;
	if (pq->size >= 2)
		return ;
	pq->nodes[pq->size].coder_id = coder_id;
	pq->nodes[pq->size].priority = priority;
	heapify_up(pq, pq->size);
	pq->size++;
}

void	dongle_request_queue_remove_coder(void *queue, int coder_id)
{
	t_priority_queue	*pq;
	int					i;

	if (!queue)
		return ;
	pq = (t_priority_queue *)queue;
	i = 0;
	while (i < pq->size)
	{
		if (pq->nodes[i].coder_id == coder_id)
		{
			pq->size--;
			if (i < pq->size)
				pq->nodes[i] = pq->nodes[pq->size];
			if (pq->size == 0)
				return ;
			if (i == 0)
				heapify_down(pq, 0);
			else if (i < pq->size)
				heapify_up(pq, i);
			return ;
		}
		i++;
	}
}

int	dongle_request_queue_remove_front(void *queue)
{
	t_priority_queue	*pq;
	int					coder_id;

	if (!queue)
		return (-1);
	pq = (t_priority_queue *)queue;
	if (pq->size == 0)
		return (-1);
	coder_id = pq->nodes[0].coder_id;
	pq->size--;
	if (pq->size > 0)
	{
		pq->nodes[0] = pq->nodes[pq->size];
		heapify_down(pq, 0);
	}
	return (coder_id);
}
