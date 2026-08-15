/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_queue.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:44:35 by atinoco-          #+#    #+#             */
/*   Updated: 2026/08/14 01:20:48 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
static int	sched_debug_enabled(void)
{
	static int	cached = -1;

	if (cached == -1)
		cached = (getenv("CODEXION_DEBUG") != NULL);
	return (cached);
} */

/* CODEXION_DEBUG=1 ./codexion 5 3000 200 200 200 3 800 edf 2>debug.txt
static void	log_sched_decision(t_dongle *dongle, t_sim *sim)
{
	long long	elapsed;
	long long	start_ms;

	if (!sched_debug_enabled() || dongle->queue_size != 2)
		return ;
	pthread_mutex_lock(&sim->log_lock);
	pthread_mutex_lock(&sim->sim_lock);
	start_ms = sim->start_time_ms;
	elapsed = current_time_ms() - start_ms;
	pthread_mutex_unlock(&sim->sim_lock);
	if (sim->config.scheduler == SCHED_POLICY_FIFO)
		fprintf(stderr,
			"%lld [sched:fifo] dongle %d: serving coder %d (arrival %lld)"
			" over coder %d (arrival %lld)\n",
			elapsed, dongle->id,
			dongle->queue[0].coder_id,
			dongle->queue[0].arrival_time - start_ms,
			dongle->queue[1].coder_id,
			dongle->queue[1].arrival_time - start_ms);
	else
		fprintf(stderr,
			"%lld [sched:edf]  dongle %d: serving coder %d (deadline %lld)"
			" over coder %d (deadline %lld)\n",
			elapsed, dongle->id,
			dongle->queue[0].coder_id,
			dongle->queue[0].deadline - start_ms,
			dongle->queue[1].coder_id,
			dongle->queue[1].deadline - start_ms);
	pthread_mutex_unlock(&sim->log_lock);
} */

/* Insert a waiting coder according to FIFO ordering. */
static void	enqueue_fifo(t_dongle *dongle, t_coder *coder)
{
	t_request	tmp;

	dongle->queue[dongle->queue_size].coder_id = coder->id;
	dongle->queue[dongle->queue_size].arrival_time = coder->arrival_time;
	dongle->queue_size++;
	if (dongle->queue_size == 2
		&& dongle->queue[0].arrival_time > dongle->queue[1].arrival_time)
	{
		tmp = dongle->queue[0];
		dongle->queue[0] = dongle->queue[1];
		dongle->queue[1] = tmp;
	}
}

/* Insert a waiting coder according to EDF ordering. */
static void	enqueue_edf(t_dongle *dongle, t_coder *coder)
{
	t_request	tmp;
	long long	deadline;

	pthread_mutex_lock(&coder->sim->sim_lock);
	deadline = coder->last_compile_start
		+ coder->sim->config.time_to_burnout;
	pthread_mutex_unlock(&coder->sim->sim_lock);
	dongle->queue[dongle->queue_size].coder_id = coder->id;
	dongle->queue[dongle->queue_size].deadline = deadline;
	dongle->queue_size++;
	if (dongle->queue_size == 2)
	{
		if (dongle->queue[0].deadline > dongle->queue[1].deadline
			|| (dongle->queue[0].deadline
				== dongle->queue[1].deadline
				&& dongle->queue[0].coder_id
				> dongle->queue[1].coder_id))
		{
			tmp = dongle->queue[0];
			dongle->queue[0] = dongle->queue[1];
			dongle->queue[1] = tmp;
		}
	}
}

/* Insert one waiting coder into this dongle's queue. */
/* log_sched_decision(dongle, coder->sim); */
void	enqueue(t_dongle *dongle, t_coder *coder)
{
	if (coder->sim->config.scheduler == SCHED_POLICY_FIFO)
		enqueue_fifo(dongle, coder);
	else
		enqueue_edf(dongle, coder);
}

/* Remove the front request from the queue. */
void	dequeue(t_dongle *dongle)
{
	if (dongle->queue_size <= 0)
		return ;
	dongle->queue[0] = dongle->queue[1];
	dongle->queue_size--;
}

/* Wait until this coder is first in line and the dongle is ready. */
void	wait_for_dongle(t_dongle *dongle, t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&dongle->lock);
	enqueue(dongle, coder);
	while (sim_is_running(sim))
	{
		if (dongle->available
			&& dongle->queue[0].coder_id == coder->id
			&& current_time_ms() - dongle->timestamp
			>= sim->config.dongle_cooldown)
			break ;
		pthread_mutex_unlock(&dongle->lock);
		usleep(100);
		pthread_mutex_lock(&dongle->lock);
	}
	if (!sim_is_running(sim))
	{
		dequeue(dongle);
		pthread_mutex_unlock(&dongle->lock);
		return ;
	}
	dongle->available = 0;
	dequeue(dongle);
	pthread_mutex_unlock(&dongle->lock);
}
