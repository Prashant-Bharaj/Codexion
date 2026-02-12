/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_dongle.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prasingh <prasingh@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by prasingh          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by prasingh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdlib.h>
#include <unistd.h>

static void	order_indices(int left, int right, int *first, int *second)
{
	if (left <= right)
	{
		*first = left;
		*second = right;
	}
	else
	{
		*first = right;
		*second = left;
	}
}

static long	get_deadline(t_coder_data *data, long time_to_burnout)
{
	long	deadline;

	pthread_mutex_lock(&data->mutex);
	deadline = data->last_compile_start + time_to_burnout;
	pthread_mutex_unlock(&data->mutex);
	return (deadline);
}

int	acquire_two_dongles(t_sim *sim, int coder_id, int left_idx, int right_idx)
{
	int		first;
	int		second;
	long	deadline;

	if (sim->params.num_coders < 2)
		return (-1);
	deadline = get_deadline(&sim->coder_data[coder_id - 1],
			sim->params.time_to_burnout);
	order_indices(left_idx, right_idx, &first, &second);
	if (dongle_acquire(&sim->dongles[first], sim, coder_id, deadline) != 0)
		return (-1);
	safe_log(sim, coder_id, "has taken a dongle");
	if (first != second && dongle_acquire(&sim->dongles[second], sim, coder_id,
			deadline) != 0)
	{
		dongle_release(&sim->dongles[first], sim);
		return (-1);
	}
	if (first != second)
		safe_log(sim, coder_id, "has taken a dongle");
	return (0);
}

void	release_two_dongles(t_sim *sim, int left_idx, int right_idx)
{
	int	first;
	int	second;

	order_indices(left_idx, right_idx, &first, &second);
	dongle_release(&sim->dongles[first], sim);
	if (first != second)
		dongle_release(&sim->dongles[second], sim);
}
