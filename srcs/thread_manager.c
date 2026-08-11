/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread_manager.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 23:44:37 by atinoco-          #+#    #+#             */
/*   Updated: 2026/08/04 13:11:23 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Wait at the common start barrier, then run the coder's loop. */
static void	*coder_entry(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;
	pthread_mutex_lock(&sim->start_lock);
	while (!sim->start_released && !sim->start_aborted)
		pthread_cond_wait(&sim->start_cond, &sim->start_lock);
	pthread_mutex_unlock(&sim->start_lock);
	if (sim->start_aborted)
		return (NULL);
	pthread_mutex_lock(&sim->start_lock);
	sim->registered_coders++;
	if (sim->registered_coders == sim->config.num_coders)
		pthread_cond_broadcast(&sim->start_cond);
	else
	{
		while (sim->registered_coders < sim->config.num_coders)
			pthread_cond_wait(&sim->start_cond, &sim->start_lock);
	}
	pthread_mutex_unlock(&sim->start_lock);
	return (coder_loop_run(coder));
}

/* Create a thread to each coder, tracking progress for cleanup. */
static int	spawn_coders(t_sim *sim)
{
	int	i;
	int	ret;

	i = 0;
	while (i < sim->config.num_coders)
	{
		sim->coders[i].sim = sim;
		ret = pthread_create(&sim->coders[i].thread, NULL,
				coder_entry, &sim->coders[i]);
		if (ret != 0)
		{
			fprintf(stderr,
				"Error: failed to create thread %d (pthread_create: %d)\n",
				i + 1, ret);
			return (0);
		}
		sim->coders[i].thread_created = 1;
		sim->threads_created++;
		i++;
	}
	return (1);
}

/* Create the monitor thread. */
static int	spawn_monitor(t_sim *sim)
{
	if (pthread_create(&sim->monitor, NULL, watchdog_run, sim) != 0)
		return (0);
	sim->monitor_created = 1;
	return (1);
}

/* Release the barrier normally, or abort it on failure. */
static void	release_barrier(t_sim *sim, int aborted)
{
	long long	now;
	int			i;

	now = current_time_ms();
	pthread_mutex_lock(&sim->start_lock);
	if (aborted)
		sim->start_aborted = 1;
	else
	{
		sim->start_time_ms = now;
		i = 0;
		while (i < sim->config.num_coders)
			sim->coders[i++].last_compile_start = now;
		sim->start_released = 1;
	}
	pthread_cond_broadcast(&sim->start_cond);
	pthread_mutex_unlock(&sim->start_lock);
}

/* Spawn all coder threads and the monitor, then release the barrier. */
int	thread_manager_start(t_sim *sim)
{
	if (!spawn_coders(sim))
	{
		release_barrier(sim, 1);
		return (0);
	}
	if (!spawn_monitor(sim))
	{
		release_barrier(sim, 1);
		return (0);
	}
	release_barrier(sim, 0);
	return (1);
}
