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

void	heapify_up(t_priority_queue *pq, int idx)
{
	t_pq_node	tmp;

	if (idx == 1 && pq->nodes[1].priority < pq->nodes[0].priority)
	{
		tmp = pq->nodes[0];
		pq->nodes[0] = pq->nodes[1];
		pq->nodes[1] = tmp;
	}
}

void	heapify_down(t_priority_queue *pq, int idx)
{
	t_pq_node	tmp;

	if (idx == 0 && pq->size > 1
		&& pq->nodes[1].priority < pq->nodes[0].priority)
	{
		tmp = pq->nodes[0];
		pq->nodes[0] = pq->nodes[1];
		pq->nodes[1] = tmp;
	}
}
