/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_grab.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/24 10:48:26 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/24 10:48:26 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <sys/time.h>

/* Wait for a dongle-release event or up to 5ms, whichever is first. Bounded 
/* wait keeps us responsive to stop/burnout, while pthread_cond_broadcast 
/* wakes us immediately whenever a dongle becomes free. */
void	dongle_cond_wait(t_sim *sim)
{
	struct 			timespec	ts;
	long long		target;

	target = current_time_ms() + 5;
	ts.tv_sec = (time_t)(target / 1000);
	ts.tv_nsec = (long)(target % 1000) * 1000000L;
	pthread_mutex_lock(&sim->sim_lock);
	pthread_cond_timedwait(&sim->dongle_cond, &sim->sim_lock, &ts);
	pthread_mutex_unlock(&sim->sim_lock);
}

/* True only if both dongles are free AND the scheduler currently */
/* picks this coder as the rightful next holder for both of them. */
/* Caller must hold f->lock, s->lock, and sim->sim_lock. */
static int	pair_ready(t_coder *coder, t_sim *sim, t_dongle *f, t_dongle *s)
{
	long long	now;

	if (!sim->running)
		return (0);
	if (f->is_taken || s->is_taken)
		return (0);
	now = current_time_ms();
	if (now < f->available_at_ms || now < s->available_at_ms)
		return (0);
	if (!scheduler_is_selected(sim, f, coder))
		return (0);
	return (scheduler_is_selected(sim, s, coder));
}

/* Single-coder case: only one dongle exists (left == right). */
/* The coder can never compile; it just waits until burnout. */
void	single_coder_wait(t_coder *coder, t_sim *sim)
{
	while (coder_should_continue(coder))
		dongle_cond_wait(sim);
}

/* Try to take both dongles atomically; retry until success or stop. */
int	acquire_both_dongles(t_sim *sim, t_coder *coder, t_dongle *f,
		t_dongle *s)
{
	scheduler_register_wait(sim, coder, f, s);
	while (sim->running)
	{
		pthread_mutex_lock(&f->lock);
		pthread_mutex_lock(&s->lock);
		pthread_mutex_lock(&sim->sim_lock);
		if (pair_ready(coder, sim, f, s))
		{
			f->is_taken = 1;
			s->is_taken = 1;
			pthread_mutex_unlock(&sim->sim_lock);
			pthread_mutex_unlock(&s->lock);
			pthread_mutex_unlock(&f->lock);
			scheduler_clear_wait(sim, coder);
			log_state(sim, coder->id, "has taken a dongle");
			log_state(sim, coder->id, "has taken a dongle");
			return (1);
		}
		pthread_mutex_unlock(&sim->sim_lock);
		pthread_mutex_unlock(&s->lock);
		pthread_mutex_unlock(&f->lock);
		dongle_cond_wait(sim);
	}
	scheduler_clear_wait(sim, coder);
	return (0);
}
