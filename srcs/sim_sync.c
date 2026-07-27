/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_sync.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 10:51:35 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/21 10:51:35 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Init the 3 simulation mutexes. Rolls back on partial failure. */
int	init_sim_sync(t_sim *sim)
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

/* Destroy the 3 simulation mutexes and the start condition. */
void	destroy_sim_sync(t_sim *sim)
{
	pthread_cond_destroy(&sim->start_cond);
	pthread_mutex_destroy(&sim->start_lock);
	pthread_mutex_destroy(&sim->log_lock);
	pthread_mutex_destroy(&sim->sim_lock);
	sim->mutexes_ready = 0;
}

/* Set up one coder's fields and its left/right dongle pointers. */
static void	init_one_coder(t_sim *sim, int i)
{
	sim->coders[i].id = i + 1;
	sim->coders[i].compiles_done = 0;
	sim->coders[i].last_compile_start = 0;
	sim->coders[i].ticket = 0;
	sim->coders[i].thread_created = 0;
	sim->coders[i].left = &sim->dongles[i];
	sim->coders[i].right = &sim->dongles[(i + 1) % sim->config.num_coders];
}

/* Init every coder and every dongle mutex. Tracks progress */
/* in sim->dongles_ready for safe partial rollback on failure. */
int	init_coders_and_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.num_coders)
	{
		init_one_coder(sim, i);
		sim->dongles[i].id = i + 1;
		sim->dongles[i].is_taken = 0;
		sim->dongles[i].available_at_ms = 0;
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL) != 0)
			return (0);
		sim->dongles_ready++;
		i++;
	}
	return (1);
}

/* Destroy the first `count` dongle mutexes (partial-safe). */
void	destroy_dongle_locks(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_mutex_destroy(&sim->dongles[i].lock);
		i++;
	}
	sim->dongles_ready = 0;
}
