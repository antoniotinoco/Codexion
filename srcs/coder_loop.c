/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_loop.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 16:59:46 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/22 16:59:46 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Pick this coder's two dongles, lower-address first. */
/* Lower-first ordering breaks circular wait (deadlock prevention). */
static void	choose_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->left < coder->right)
	{
		*first = coder->left;
		*second = coder->right;
	}
	else
	{
		*first = coder->right;
		*second = coder->left;
	}
}

/* Run one compile cycle: acquire both dongles, hold, release. */
static void	run_compile(t_coder *coder, t_sim *sim)
{
	t_dongle	*first;
	t_dongle	*second;

	if (sim->config.num_coders == 1)
		return (single_coder_wait(coder, sim));
	choose_dongles(coder, &first, &second);
	if (!acquire_both_dongles(sim, coder, first, second))
		return ;
	pthread_mutex_lock(&sim->sim_lock);
	coder->last_compile_start = current_time_ms();
	pthread_mutex_unlock(&sim->sim_lock);
	log_state(sim, coder->id, "is compiling");
	sleep_with_stop(sim, sim->config.time_to_compile);
	release_dongle(sim, second);
	release_dongle(sim, first);
	if (coder_should_continue(coder))
		update_compile_state(coder, sim);
}

/* Run the debug step. */
static void	run_debug(t_coder *coder, t_sim *sim)
{
	log_state(sim, coder->id, "is debugging");
	sleep_with_stop(sim, sim->config.time_to_debug);
}

/* Run the refactor step. */
static void	run_refactor(t_coder *coder, t_sim *sim)
{
	log_state(sim, coder->id, "is refactoring");
	sleep_with_stop(sim, sim->config.time_to_refactor);
}

/* Full life cycle loop for one coder thread. */
void	*coder_loop_run(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	while (coder_should_continue(coder))
	{
		run_compile(coder, sim);
		if (!coder_should_continue(coder))
			break ;
		run_debug(coder, sim);
		if (!coder_should_continue(coder))
			break ;
		run_refactor(coder, sim);
	}
	return (NULL);
}
