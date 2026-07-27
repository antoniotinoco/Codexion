/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim_state.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 23:48:04 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/23 23:48:04 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <unistd.h>
#include <sys/time.h>

/* Current time in milliseconds since an arbitrary fixed point. */
long long	current_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long long)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/* Return whether the simulation is still running (thread-safe read). */
int	sim_is_running(t_sim *sim)
{
	int	running;

	pthread_mutex_lock(&sim->sim_lock);
	running = sim->running;
	pthread_mutex_unlock(&sim->sim_lock);
	return (running);
}

/* A coder keeps looping only while the simulation is running. */
/* Goal / burnout detection is fully owned by the monitor thread. */
int	coder_should_continue(t_coder *coder)
{
	return (sim_is_running(coder->sim));
}

/* Record a finished compile: bump the counter, reset the deadline. */
void	update_compile_state(t_coder *coder, t_sim *sim)
{
	pthread_mutex_lock(&sim->sim_lock);
	coder->compiles_done++;
	pthread_mutex_unlock(&sim->sim_lock);
}

/* Sleep in small steps so a stop signal is noticed quickly. */
void	sleep_with_stop(t_sim *sim, long long ms)
{
	long long	start;
	long long	remaining;

	start = current_time_ms();
	while (sim_is_running(sim))
	{
		remaining = ms - (current_time_ms() - start);
		if (remaining <= 0)
			break ;
		if (remaining > 50)
			usleep(10000);
		else if (remaining > 20)
			usleep(2000);
		else
			usleep(200);
	}
}
