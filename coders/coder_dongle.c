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

static long	compute_priority(t_sim *sim, int cid)
{
	long	deadline;

	pthread_mutex_lock(&sim->coder_data[cid - 1].mutex);
	deadline = sim->coder_data[cid - 1].last_compile_start
		+ sim->params.time_to_burnout;
	pthread_mutex_unlock(&sim->coder_data[cid - 1].mutex);
	if (sim->params.scheduler == CODEX_FIFO)
		return (get_time_ms());
	return (deadline * (sim->params.num_coders + 1) - cid);
}

static int	phase1_take_f(t_sim *sim, int cid, int f, long priority)
{
	long			now;
	long			wait_ms;
	struct timespec	ts;

	pthread_mutex_lock(&sim->dongles[f].mutex);
	dongle_request_queue_add(sim->dongles[f].request_queue, cid, priority);
	while (1)
	{
		now = get_time_ms();
		if (now >= sim->dongles[f].cooldown_until
			&& sim->dongles[f].holder < 0
			&& dongle_request_queue_peek_can_serve(
				sim->dongles[f].request_queue, cid))
			break ;
		if (is_stopped(sim))
		{
			dongle_request_queue_remove_coder(sim->dongles[f].request_queue,
				cid);
			pthread_mutex_unlock(&sim->dongles[f].mutex);
			return (-1);
		}
		wait_ms = pair_wait_ms(sim, f, f, now);
		abs_time_in_ms(wait_ms, &ts);
		pthread_cond_timedwait(&sim->dongles[f].cond,
			&sim->dongles[f].mutex, &ts);
	}
	dongle_request_queue_remove_front(sim->dongles[f].request_queue);
	sim->dongles[f].holder = cid;
	pthread_mutex_unlock(&sim->dongles[f].mutex);
	return (0);
}

static int	phase2_take_s(t_sim *sim, int cid, int f, int s, long priority)
{
	long			now;
	long			wait_ms;
	struct timespec	ts;

	(void)f;
	pthread_mutex_lock(&sim->dongles[s].mutex);
	dongle_request_queue_add(sim->dongles[s].request_queue_s, cid, priority);
	while (1)
	{
		now = get_time_ms();
		if (now >= sim->dongles[s].cooldown_until
			&& sim->dongles[s].holder < 0
			&& dongle_request_queue_peek_can_serve(
				sim->dongles[s].request_queue_s, cid))
			break ;
		if (is_stopped(sim))
		{
			dongle_request_queue_remove_coder(
				sim->dongles[s].request_queue_s, cid);
			pthread_mutex_unlock(&sim->dongles[s].mutex);
			return (-1);
		}
		wait_ms = pair_wait_ms(sim, f, s, now);
		abs_time_in_ms(wait_ms, &ts);
		pthread_cond_timedwait(&sim->dongles[s].cond,
			&sim->dongles[s].mutex, &ts);
	}
	dongle_request_queue_remove_front(sim->dongles[s].request_queue_s);
	sim->dongles[s].holder = cid;
	pthread_mutex_unlock(&sim->dongles[s].mutex);
	return (0);
}

int	acquire_two_dongles(t_sim *sim, int cid, int left, int right)
{
	int		f;
	int		s;
	long	priority;

	if (sim->params.num_coders < 2)
		return (-1);
	order_indices(left, right, &f, &s);
	priority = compute_priority(sim, cid);
	if (phase1_take_f(sim, cid, f, priority) != 0)
		return (-1);
	if (phase2_take_s(sim, cid, f, s, priority) != 0)
	{
		pthread_mutex_lock(&sim->dongles[f].mutex);
		sim->dongles[f].holder = -1;
		pthread_mutex_unlock(&sim->dongles[f].mutex);
		wake_all_dongles(sim);
		return (-1);
	}
	safe_log(sim, cid, "has taken a dongle");
	safe_log(sim, cid, "has taken a dongle");
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
