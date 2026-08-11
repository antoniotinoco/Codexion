/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_sync.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 15:51:35 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/23 20:52:04 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Destroy sync objects and free simulation memory. Safe to call */
/* after partial init/start failures at any stage. */
void	thread_manager_cleanup(t_sim *sim)
{
	if (sim->dongles_ready > 0)
		destroy_dongle_locks(sim, sim->dongles_ready);
	if (sim->mutexes_ready)
		destroy_sim_sync(sim);
	free_sim_arrays(sim);
}

/* Destroy the simulation mutexes. */
void	destroy_sim_sync(t_sim *sim)
{
	pthread_cond_destroy(&sim->start_cond);
	pthread_mutex_destroy(&sim->start_lock);
	pthread_mutex_destroy(&sim->log_lock);
	pthread_mutex_destroy(&sim->sim_lock);
	sim->mutexes_ready = 0;
}

/* Destroy initialized dongles. */
void	destroy_dongle_locks(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_cond_destroy(&sim->dongles[i].cond);
		pthread_mutex_destroy(&sim->dongles[i].lock);
		i++;
	}
	sim->dongles_ready = 0;
}

/* Undo whatever init_sim_sync / init_coders_and_dongles did. */
void	cleanup_partial_setup(t_sim *sim)
{
	if (sim->dongles_ready > 0)
		destroy_dongle_locks(sim, sim->dongles_ready);
	if (sim->mutexes_ready)
		destroy_sim_sync(sim);
	free_sim_arrays(sim);
}

/* Free every heap-allocated array in t_sim. */
void	free_sim_arrays(t_sim *sim)
{
	free(sim->coders);
	free(sim->dongles);
	sim->coders = NULL;
	sim->dongles = NULL;
}
