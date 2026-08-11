/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 10:45:11 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/23 13:55:01 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Initialize one coder. */
static void	init_one_coder(t_sim *sim, int i)
{
	sim->coders[i].id = i + 1;
	sim->coders[i].compiles_done = 0;
	sim->coders[i].last_compile_start = 0;
	sim->coders[i].arrival_time = 0;
	sim->coders[i].thread_created = 0;
	sim->coders[i].left = &sim->dongles[i];
	sim->coders[i].right = &sim->dongles[(i + 1) % sim->config.num_coders];
	sim->coders[i].sim = sim;
}

/* Initialize coders and dongles. */
static int	init_coders_and_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.num_coders)
	{
		init_one_coder(sim, i);
		sim->dongles[i].id = i + 1;
		sim->dongles[i].available = 1;
		sim->dongles[i].timestamp = 0;
		sim->dongles[i].queue_size = 0;
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL) != 0)
			return (0);
		if (pthread_cond_init(&sim->dongles[i].cond, NULL) != 0)
		{
			pthread_mutex_destroy(&sim->dongles[i].lock);
			return (0);
		}
		sim->dongles_ready++;
		i++;
	}
	return (1);
}

/* Init the simulation mutexes. Rolls back on partial failure. */
static int	init_sim_sync(t_sim *sim)
{
	if (pthread_mutex_init(&sim->sim_lock, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->log_lock, NULL) != 0)
		return (pthread_mutex_destroy(&sim->sim_lock), 0);
	if (pthread_mutex_init(&sim->start_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->log_lock);
		pthread_mutex_destroy(&sim->sim_lock);
		return (0);
	}
	if (pthread_cond_init(&sim->start_cond, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->start_lock);
		pthread_mutex_destroy(&sim->log_lock);
		pthread_mutex_destroy(&sim->sim_lock);
		return (0);
	}
	sim->mutexes_ready = 1;
	return (1);
}

/* Allocate coder and dongle arrays. */
static int	alloc_sim_arrays(t_sim *sim)
{
	int	n;

	n = sim->config.num_coders;
	sim->coders = malloc(sizeof(t_coder) * n);
	sim->dongles = malloc(sizeof(t_dongle) * n);
	if (!sim->coders || !sim->dongles)
	{
		free_sim_arrays(sim);
		return (0);
	}
	return (1);
}

/* Parse args, allocate memory, init sync objects and data. */
int	setup_simulation(t_sim *sim, int argc, char **argv)
{
	memset(sim, 0, sizeof(t_sim));
	if (!parse_args(sim, argc, argv))
		return (0);
	if (!alloc_sim_arrays(sim))
		return (0);
	if (!init_sim_sync(sim))
	{
		free_sim_arrays(sim);
		return (0);
	}
	if (!init_coders_and_dongles(sim))
	{
		cleanup_partial_setup(sim);
		return (0);
	}
	sim->running = 1;
	return (1);
}
