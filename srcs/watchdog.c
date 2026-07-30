/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   watchdog.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 00:34:29 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/30 00:34:29 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <unistd.h>

/* Find a burned-out coder without modifying simulation state. */
/* Caller must hold sim->sim_lock. */
static int	find_burned_coder(t_sim *sim, long long now)
{
	int			i;
	long long	elapsed;

	i = 0;
	while (i < sim->config.num_coders)
	{
		elapsed = now - sim->coders[i].last_compile_start;
		if (elapsed > sim->config.time_to_burnout)
			return (sim->coders[i].id);
		i++;
	}
	return (0);
}

/* Detect burnout and stop the simulation with a serialized log. */
static int	detect_burnout(t_sim *sim)
{
	long long	now;
	int			burned;

	now = current_time_ms();
	pthread_mutex_lock(&sim->sim_lock);
	if (!sim->running)
	{
		pthread_mutex_unlock(&sim->sim_lock);
		return (0);
	}
	burned = find_burned_coder(sim, now);
	pthread_mutex_unlock(&sim->sim_lock);
	if (burned != 0)
	{
		log_burnout_and_stop(sim, burned);
		return (1);
	}
	return (0);
}

/* True once every coder has reached the required compile count. */
/* Caller must hold sim->sim_lock. */
static int	goal_reached_locked(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->config.num_coders)
	{
		if (sim->coders[i].compiles_done < sim->config.compiles_required)
			return (0);
		i++;
	}
	return (1);
}

/* Check whether every coder reached the compile goal and stop if so. */
static int	check_goal_complete(t_sim *sim)
{
	int	done;

	pthread_mutex_lock(&sim->sim_lock);
	if (!sim->running)
	{
		pthread_mutex_unlock(&sim->sim_lock);
		return (1);
	}
	done = goal_reached_locked(sim);
	if (done)
		sim->running = 0;
	pthread_mutex_unlock(&sim->sim_lock);
	if (done)
		pthread_cond_broadcast(&sim->dongle_cond);
	return (done);
}

/* Wait for the common start, then monitor burnout/goal completion. */
void	*watchdog_run(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	pthread_mutex_lock(&sim->start_lock);
	while (!sim->start_released && !sim->start_aborted)
		pthread_cond_wait(&sim->start_cond, &sim->start_lock);
	pthread_mutex_unlock(&sim->start_lock);
	if (sim->start_aborted)
		return (NULL);
	while (sim_is_running(sim))
	{
		if (detect_burnout(sim) || check_goal_complete(sim))
			break ;
		usleep(2000);
	}
	return (NULL);
}
