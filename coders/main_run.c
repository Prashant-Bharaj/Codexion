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

static int	start_coder_threads(t_sim *sim, pthread_t *ct, t_coder_arg *args)
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
			while (i > 0)
			{
				i--;
				pthread_join(ct[i], NULL);
			}
			return (-1);
		}
		i++;
	}
	return (0);
}

static int	start_monitor_thread(t_sim *sim, pthread_t *ct, int coder_count)
{
	int	i;

	if (pthread_create(&ct[coder_count], NULL, monitor_routine, sim) != 0)
	{
		signal_stop(sim);
		i = coder_count;
		while (i > 0)
		{
			i--;
			pthread_join(ct[i], NULL);
		}
		return (-1);
	}
	return (0);
}

static void	join_and_cleanup(t_sim *sim, pthread_t *threads, t_coder_arg *args)
{
	int	i;

	pthread_join(threads[sim->params.num_coders], NULL);
	i = 0;
	while (i < sim->params.num_coders)
	{
		pthread_join(threads[i], NULL);
		i++;
	}
	cleanup_simulation(sim);
	free(threads);
	free(args);
}

static int	init_threads_and_start(t_sim *sim,
			pthread_t **threads, t_coder_arg **args)
{
	*threads = (pthread_t *)malloc(sizeof(pthread_t)
			* (sim->params.num_coders + 1));
	*args = (t_coder_arg *)malloc(sizeof(t_coder_arg)
			* sim->params.num_coders);
	if (!*threads || !*args)
	{
		cleanup_simulation(sim);
		free(*threads);
		free(*args);
		return (-1);
	}
	if (start_coder_threads(sim, *threads, *args) != 0
		|| start_monitor_thread(sim, *threads, sim->params.num_coders) != 0)
	{
		cleanup_simulation(sim);
		free(*threads);
		free(*args);
		return (-1);
	}
	return (0);
}

int	run_simulation(t_sim *sim)
{
	pthread_t	*threads;
	t_coder_arg	*args;
	int			i;

	sim->start_time = get_time_ms();
	i = 0;
	while (i < sim->params.num_coders)
	{
		sim->coder_data[i].last_compile_start = sim->start_time;
		i++;
	}
	if (init_threads_and_start(sim, &threads, &args) != 0)
		return (-1);
	join_and_cleanup(sim, threads, args);
	return (0);
}
