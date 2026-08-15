/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   clock_log.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 13:30:26 by atinoco-          #+#    #+#             */
/*   Updated: 2026/08/10 23:59:01 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Print one serialized state line with timestamp and coder id. */
/* Suppressed once the sim has stopped, except burnout is handled */
/* by its own function below to guarantee it always prints. */
void	log_state(t_sim *sim, int coder_id, const char *state)
{
	long long	elapsed;

	pthread_mutex_lock(&sim->log_lock);
	pthread_mutex_lock(&sim->sim_lock);
	if (!sim->running)
	{
		pthread_mutex_unlock(&sim->sim_lock);
		pthread_mutex_unlock(&sim->log_lock);
		return ;
	}
	elapsed = current_time_ms() - sim->start_time_ms;
	pthread_mutex_unlock(&sim->sim_lock);
	printf("%lld %d %s\n", elapsed, coder_id, state);
	pthread_mutex_unlock(&sim->log_lock);
}

/* Stop the simulation and print the burnout log. Idempotent: */
/* only the first caller (coder or monitor) actually stops/prints. */
void	log_burnout_and_stop(t_sim *sim, int coder_id)
{
	long long	elapsed;
	int			i;

	pthread_mutex_lock(&sim->log_lock);
	pthread_mutex_lock(&sim->sim_lock);
	if (!sim->running)
	{
		pthread_mutex_unlock(&sim->sim_lock);
		pthread_mutex_unlock(&sim->log_lock);
		return ;
	}
	sim->running = 0;
	sim->burned_out_id = coder_id;
	elapsed = current_time_ms() - sim->start_time_ms;
	pthread_mutex_unlock(&sim->sim_lock);
	i = 0;
	while (i < sim->config.num_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].lock);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].lock);
		i++;
	}
	printf("%lld %d burned out\n", elapsed, coder_id);
	pthread_mutex_unlock(&sim->log_lock);
}
