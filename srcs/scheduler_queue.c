/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scheduler_queue.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 17:44:35 by atinoco-          #+#    #+#             */
/*   Updated: 2026/08/07 01:54:49 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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

/* Wait until this dongle is granted to this coder. */
void	wait_for_dongle(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->lock);
	enqueue(dongle, coder);
	while ((!dongle->available
			|| dongle->queue[0].coder_id != coder->id)
		&& sim_is_running(coder->sim))
		pthread_cond_wait(&dongle->cond, &dongle->lock);
	if (!sim_is_running(coder->sim))
	{
		dequeue(dongle);
		pthread_mutex_unlock(&dongle->lock);
		return ;
	}
	while (current_time_ms() - dongle->timestamp
		< coder->sim->config.dongle_cooldown
		&& sim_is_running(coder->sim))
	{
		pthread_mutex_unlock(&dongle->lock);
		usleep(100);
		pthread_mutex_lock(&dongle->lock);
	}
	dongle->available = 0;
	dequeue(dongle);
	pthread_mutex_unlock(&dongle->lock);
}
