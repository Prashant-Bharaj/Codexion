/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_run.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prasingh <prasingh@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 00:00:00 by prasingh          #+#    #+#             */
/*   Updated: 2026/02/08 00:00:00 by prasingh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdlib.h>

static void	join_coder_threads(pthread_t *threads, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		pthread_join(threads[i], NULL);
		i++;
	}
}

static int	start_threads(t_sim *sim, pthread_t *ct, t_coder_arg *args)
{
	int	i;

	i = 0;
	while (i < sim->params.num_coders)
	{
		args[i].sim = sim;
		args[i].coder_id = i + 1;
		if (pthread_create(&ct[i], NULL, coder_routine, &args[i]) != 0)
		{
			signal_stop(sim);
			join_coder_threads(ct, i);
			return (-1);
		}
		i++;
	}
	if (pthread_create(&ct[i], NULL, monitor_routine, sim) != 0)
	{
		signal_stop(sim);
		join_coder_threads(ct, i);
		return (-1);
	}
	return (0);
}

static void	join_and_cleanup(t_sim *sim, pthread_t *threads, t_coder_arg *args)
{
	pthread_join(threads[sim->params.num_coders], NULL);
	join_coder_threads(threads, sim->params.num_coders);
	cleanup_simulation(sim);
	free(threads);
	free(args);
}

static void	cleanup_on_fail(t_sim *sim, pthread_t *t, t_coder_arg *a)
{
	cleanup_simulation(sim);
	free(t);
	free(a);
}

int	run_simulation(t_sim *sim)
{
	pthread_t	*threads;
	t_coder_arg	*args;
	int			i;

	threads = (pthread_t *)malloc(sizeof(pthread_t)
			* (sim->params.num_coders + 1));
	args = (t_coder_arg *)malloc(sizeof(t_coder_arg) * sim->params.num_coders);
	if (!threads || !args)
	{
		cleanup_on_fail(sim, threads, args);
		return (-1);
	}
	sim->start_time = get_time_ms();
	i = 0;
	while (i < sim->params.num_coders)
	{
		sim->coder_data[i].last_compile_start = sim->start_time;
		i++;
	}
	if (start_threads(sim, threads, args) != 0)
	{
		cleanup_on_fail(sim, threads, args);
		return (-1);
	}
	join_and_cleanup(sim, threads, args);
	return (0);
}
