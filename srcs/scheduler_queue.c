/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_queue.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 01:54:49 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/27 01:54:49 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Rebuild the waiting-heap for one dongle from scratch. */
static void	rebuild_heap_locked(t_sim *sim, t_dongle *dongle)
{
	t_waiter	waiter;
	int			idx;
	int			i;

	idx = dongle->id - 1;
	sim->heap_sizes[idx] = 0;
	i = 0;
	while (i < sim->config.num_coders)
	{
		if (sim->requests[i].ticket != 0
			&& (sim->requests[i].first == dongle
				|| sim->requests[i].second == dongle))
		{
			waiter.coder_idx = i;
			waiter.ticket = sim->requests[i].ticket;
			if (sim->config.scheduler == SCHED_FIFO)
				waiter.priority = waiter.ticket;
			else
				waiter.priority = sim->coders[i].last_compile_start
					+ sim->config.time_to_burnout;
			waiter.tie = waiter.ticket;
			heap_push(sim->dongle_heaps[idx], &sim->heap_sizes[idx], waiter);
		}
		i++;
	}
}

/* Register one coder as waiting for a pair of dongles. */
void	scheduler_register_wait(t_sim *sim, t_coder *coder, t_dongle *f,
		t_dongle *s)
{
	int	idx;

	idx = coder->id - 1;
	pthread_mutex_lock(&sim->sim_lock);
	if (sim->requests[idx].ticket == 0)
		sim->requests[idx].ticket = ++sim->next_ticket;
	sim->requests[idx].first = f;
	sim->requests[idx].second = s;
	rebuild_heap_locked(sim, f);
	if (s != f)
		rebuild_heap_locked(sim, s);
	pthread_mutex_unlock(&sim->sim_lock);
}

/* Clear one coder's wait request and reset its ticket. */
void	scheduler_clear_wait(t_sim *sim, t_coder *coder)
{
	int			idx;
	t_dongle	*f;
	t_dongle	*s;

	idx = coder->id - 1;
	pthread_mutex_lock(&sim->sim_lock);
	f = sim->requests[idx].first;
	s = sim->requests[idx].second;
	sim->requests[idx].first = NULL;
	sim->requests[idx].second = NULL;
	sim->requests[idx].ticket = 0;
	if (f)
		rebuild_heap_locked(sim, f);
	if (s && s != f)
		rebuild_heap_locked(sim, s);
	pthread_cond_broadcast(&sim->dongle_cond);
	pthread_mutex_unlock(&sim->sim_lock);
}

/* Caller must already hold sim->sim_lock. */
int	scheduler_is_selected(t_sim *sim, t_dongle *dongle, t_coder *coder)
{
	int	idx;
	int	best;

	idx = dongle->id - 1;
	if (sim->heap_sizes[idx] == 0)
		return (0);
	best = heap_peek(sim->dongle_heaps[idx], sim->heap_sizes[idx]).coder_idx;
	return (best == coder->id - 1);
}
