/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_time.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prasingh <prasingh@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by prasingh          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by prasingh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <time.h>

void	abs_time_in_ms(long ms_from_now, struct timespec *ts)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	ts->tv_sec = tv.tv_sec + ms_from_now / 1000;
	ts->tv_nsec = tv.tv_usec * 1000 + (ms_from_now % 1000) * 1000000;
	if (ts->tv_nsec >= 1000000000)
	{
		ts->tv_sec++;
		ts->tv_nsec -= 1000000000;
	}
}

static long	base_wait_ms(long wait_f, long wait_s)
{
	long	wait_ms;

	if (wait_f > 0 && wait_s > 0)
	{
		if (wait_f <= wait_s)
			wait_ms = wait_f + 1;
		else
			wait_ms = wait_s + 1;
	}
	else if (wait_f > 0)
		wait_ms = wait_f + 1;
	else if (wait_s > 0)
		wait_ms = wait_s + 1;
	else
		wait_ms = 100;
	return (wait_ms);
}

long	pair_wait_ms(t_sim *sim, int f, int s, long now)
{
	long	wait_f;
	long	wait_s;
	long	wait_ms;

	wait_f = sim->dongles[f].cooldown_until - now;
	wait_s = sim->dongles[s].cooldown_until - now;
	if (wait_f < 0)
		wait_f = 0;
	if (wait_s < 0)
		wait_s = 0;
	wait_ms = base_wait_ms(wait_f, wait_s);
	if (wait_ms > 500)
		wait_ms = 500;
	if (wait_ms < 1)
		wait_ms = 1;
	return (wait_ms);
}
