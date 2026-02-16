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

static long	get_priority(t_sim *sim, long deadline)
{
	if (sim->params.scheduler == CODEX_FIFO)
		return (get_time_ms());
	return (deadline);
}

static int	pair_ready(t_sim *sim, int left_idx, int right_idx, long now)
{
	if (sim->dongles[left_idx].holder >= 0
		|| sim->dongles[right_idx].holder >= 0)
		return (0);
	if (now < sim->dongles[left_idx].cooldown_until
		|| now < sim->dongles[right_idx].cooldown_until)
		return (0);
	return (1);
}

static long	pair_wait_ms(t_sim *sim, int left_idx, int right_idx, long now)
{
	long	wait_left;
	long	wait_right;

	wait_left = calc_wait_ms(sim->dongles[left_idx].cooldown_until, now);
	wait_right = calc_wait_ms(sim->dongles[right_idx].cooldown_until, now);
	if (wait_left > wait_right)
		return (wait_left);
	return (wait_right);
}

int	acquire_two_dongles(t_sim *sim, int coder_id, int left_idx, int right_idx)
{
	long			deadline;
	long			priority;
	long			now;
	long			wait_ms;
	int			stop;
	struct timespec	abstime;

	if (sim->params.num_coders < 2)
		return (-1);
	deadline = get_deadline(&sim->coder_data[coder_id - 1],
			sim->params.time_to_burnout);
	priority = get_priority(sim, deadline);
	pthread_mutex_lock(&sim->sched_mutex);
	dongle_request_queue_add(sim->sched_queue, coder_id, priority);
	while (1)
	{
		pthread_mutex_lock(&sim->stop_mutex);
		stop = sim->stop;
		pthread_mutex_unlock(&sim->stop_mutex);
		if (stop)
		{
			pthread_mutex_unlock(&sim->sched_mutex);
			return (-1);
		}
		now = get_time_ms();
		if (dongle_request_queue_peek_can_serve(sim->sched_queue, coder_id)
			&& pair_ready(sim, left_idx, right_idx, now))
		{
			dongle_request_queue_remove_front(sim->sched_queue);
			sim->dongles[left_idx].holder = coder_id;
			sim->dongles[left_idx].cooldown_until = 0;
			sim->dongles[right_idx].holder = coder_id;
			sim->dongles[right_idx].cooldown_until = 0;
			pthread_mutex_unlock(&sim->sched_mutex);
			safe_log(sim, coder_id, "has taken a dongle");
			safe_log(sim, coder_id, "has taken a dongle");
			return (0);
		}
		wait_ms = pair_wait_ms(sim, left_idx, right_idx, now);
		abs_time_in_ms(wait_ms, &abstime);
		pthread_cond_timedwait(&sim->sched_cond, &sim->sched_mutex, &abstime);
	}
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
