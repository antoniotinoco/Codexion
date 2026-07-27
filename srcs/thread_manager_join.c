/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   thread_manager_join.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 14:59:36 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/22 14:59:36 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Join every coder thread that was actually created. */
static int	join_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->threads_created)
	{
		if (pthread_join(sim->coders[i].thread, NULL) != 0)
			return (0);
		i++;
	}
	return (1);
}

/* Join all coder threads, then the monitor thread. */
int	thread_manager_join(t_sim *sim)
{
	if (!join_coders(sim))
		return (0);
	if (sim->monitor_created)
		if (pthread_join(sim->monitor, NULL) != 0)
			return (0);
	return (1);
}

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
