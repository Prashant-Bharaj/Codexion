/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pq_heap.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prasingh <prasingh@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by prasingh          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by prasingh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdlib.h>

void	heapify_up(t_priority_queue *pq, int idx)
{
	int			parent;
	t_pq_node	tmp;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (pq->nodes[idx].priority >= pq->nodes[parent].priority)
			break ;
		tmp = pq->nodes[idx];
		pq->nodes[idx] = pq->nodes[parent];
		pq->nodes[parent] = tmp;
		idx = parent;
	}
}

void	heapify_down(t_priority_queue *pq, int idx)
{
	int			left;
	int			right;
	int			smallest;
	t_pq_node	tmp;

	while (1)
	{
		left = 2 * idx + 1;
		right = 2 * idx + 2;
		smallest = idx;
		if (left < pq->size
			&& pq->nodes[left].priority < pq->nodes[smallest].priority)
			smallest = left;
		if (right < pq->size
			&& pq->nodes[right].priority < pq->nodes[smallest].priority)
			smallest = right;
		if (smallest == idx)
			break ;
		tmp = pq->nodes[idx];
		pq->nodes[idx] = pq->nodes[smallest];
		pq->nodes[smallest] = tmp;
		idx = smallest;
	}
}

int	grow_queue(t_priority_queue *pq)
{
	t_pq_node	*nodes;
	int			i;

	nodes = (t_pq_node *)malloc(sizeof(t_pq_node) * pq->capacity * 2);
	if (!nodes)
		return (-1);
	i = 0;
	while (i < pq->size)
	{
		nodes[i] = pq->nodes[i];
		i++;
	}
	free(pq->nodes);
	pq->nodes = nodes;
	pq->capacity *= 2;
	return (0);
}
