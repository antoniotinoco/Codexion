/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   setup.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 10:45:11 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/21 10:45:11 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <string.h>
#include <stdlib.h>

/* Free every heap-allocated array in t_sim. */
void	free_sim_arrays(t_sim *sim)
{
	int	i;

	free(sim->coders);
	free(sim->dongles);
	free(sim->requests);
	if (sim->dongle_heaps)
	{
		i = 0;
		while (i < sim->config.num_coders)
			free(sim->dongle_heaps[i++]);
	}
	free(sim->dongle_heaps);
	free(sim->heap_sizes);
	sim->coders = NULL;
	sim->dongles = NULL;
	sim->requests = NULL;
	sim->dongle_heaps = NULL;
	sim->heap_sizes = NULL;
}

/* Undo whatever init_sim_sync / init_coders_and_dongles did. */
static void	cleanup_partial_setup(t_sim *sim)
{
	if (sim->dongles_ready > 0)
		destroy_dongle_locks(sim, sim->dongles_ready);
	if (sim->mutexes_ready)
		destroy_sim_sync(sim);
	free_sim_arrays(sim);
}

/* Allocate coders, dongles, requests, heaps and heap-size arrays. */
static int	alloc_sim_arrays(t_sim *sim)
{
	int	n;

	n = sim->config.num_coders;
	sim->coders = malloc(sizeof(t_coder) * n);
	sim->dongles = malloc(sizeof(t_dongle) * n);
	sim->requests = malloc(sizeof(t_request) * n);
	sim->heap_sizes = malloc(sizeof(int) * n);
	if (!sim->coders || !sim->dongles || !sim->requests
		|| !sim->heap_sizes || !alloc_heaps(sim, n))
	{
		free_sim_arrays(sim);
		return (0);
	}
	return (1);
}

/* Allocate the per-coder heaps array (one t_waiter array per dongle). */
static int	alloc_heaps(t_sim *sim, int n)
{
	int	i;

	sim->dongle_heaps = malloc(sizeof(t_waiter *) * n);
	if (!sim->dongle_heaps)
		return (0);
	i = 0;
	while (i < n)
		sim->dongle_heaps[i++] = NULL;
	i = 0;
	while (i < n)
	{
		sim->dongle_heaps[i] = malloc(sizeof(t_waiter) * n);
		if (!sim->dongle_heaps[i])
			return (0);
		i++;
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
