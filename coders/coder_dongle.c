/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_dongle.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prasingh <prasingh@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by prasingh          #+#    #+#             */
/*   Updated: 2026/02/17 20:22:28 by prasingh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	enqueue_both(t_sim *sim, int cid, int f, int s)
{
	long	deadline;
	long	priority;

	pthread_mutex_lock(&sim->coder_data[cid - 1].mutex);
	deadline = sim->coder_data[cid - 1].last_compile_start
		+ sim->params.time_to_burnout;
	pthread_mutex_unlock(&sim->coder_data[cid - 1].mutex);
	if (sim->params.scheduler == CODEX_FIFO)
		priority = get_time_ms();
	else
		priority = deadline;
	pthread_mutex_lock(&sim->dongles[f].mutex);
	dongle_request_queue_add(sim->dongles[f].request_queue,
		cid, priority);
	pthread_mutex_unlock(&sim->dongles[f].mutex);
	pthread_mutex_lock(&sim->dongles[s].mutex);
	dongle_request_queue_add(sim->dongles[s].request_queue,
		cid, priority);
	pthread_mutex_unlock(&sim->dongles[s].mutex);
}

static int	try_take_pair(t_sim *sim, int cid, int f, int s)
{
	long	now;

	now = get_time_ms();
	if (now < sim->dongles[f].cooldown_until
		|| sim->dongles[f].holder >= 0
		|| !dongle_request_queue_peek_can_serve(
			sim->dongles[f].request_queue, cid))
		return (0);
	if (now < sim->dongles[s].cooldown_until
		|| sim->dongles[s].holder >= 0
		|| !dongle_request_queue_peek_can_serve(
			sim->dongles[s].request_queue, cid))
		return (0);
	dongle_request_queue_remove_front(sim->dongles[f].request_queue);
	dongle_request_queue_remove_front(sim->dongles[s].request_queue);
	sim->dongles[f].holder = cid;
	sim->dongles[s].holder = cid;
	return (1);
}

static int	acquire_loop(t_sim *sim, int cid, int f, int s)
{
	long			now;
	long			wait_ms;
	struct timespec	ts;

	pthread_mutex_lock(&sim->dongles[f].mutex);
	while (1)
	{
		pthread_mutex_lock(&sim->dongles[s].mutex);
		if (try_take_pair(sim, cid, f, s))
		{
			pthread_mutex_unlock(&sim->dongles[s].mutex);
			pthread_mutex_unlock(&sim->dongles[f].mutex);
			safe_log(sim, cid, "has taken a dongle");
			safe_log(sim, cid, "has taken a dongle");
			return (0);
		}
		now = get_time_ms();
		wait_ms = pair_wait_ms(sim, f, s, now);
		pthread_mutex_unlock(&sim->dongles[s].mutex);
		abs_time_in_ms(wait_ms, &ts);
		pthread_cond_timedwait(&sim->dongles[f].cond,
			&sim->dongles[f].mutex, &ts);
		if (is_stopped(sim))
			return (pthread_mutex_unlock(&sim->dongles[f].mutex), (-1));
	}
}

int	acquire_two_dongles(t_sim *sim, int cid, int left, int right)
{
	int	f;
	int	s;

	if (sim->params.num_coders < 2)
		return (-1);
	order_indices(left, right, &f, &s);
	enqueue_both(sim, cid, f, s);
	return (acquire_loop(sim, cid, f, s));
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
