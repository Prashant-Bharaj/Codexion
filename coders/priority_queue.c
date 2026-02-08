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
	pq->capacity = 64;
	pq->size = 0;
	pq->scheduler = scheduler;
	pq->nodes = (t_pq_node *)malloc(sizeof(t_pq_node) * pq->capacity);
	if (!pq->nodes)
	{
		free(pq);
		return (NULL);
	}
	return (pq);
}

void	dongle_request_queue_destroy(void *queue)
{
	t_priority_queue	*pq;

	if (!queue)
		return ;
	pq = (t_priority_queue *)queue;
	free(pq->nodes);
	free(pq);
}

void	dongle_request_queue_add(void *queue, int coder_id, long priority)
{
	t_priority_queue	*pq;

	if (!queue)
		return ;
	pq = (t_priority_queue *)queue;
	if (pq->size >= pq->capacity && grow_queue(pq) != 0)
		return ;
	pq->nodes[pq->size].coder_id = coder_id;
	pq->nodes[pq->size].priority = priority;
	heapify_up(pq, pq->size);
	pq->size++;
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

int	dongle_request_queue_peek_can_serve(void *queue, int coder_id)
{
	t_priority_queue	*pq;

	if (!queue)
		return (0);
	pq = (t_priority_queue *)queue;
	if (pq->size == 0)
		return (0);
	return (pq->nodes[0].coder_id == coder_id);
}
