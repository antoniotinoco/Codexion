/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_loop.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/26 16:59:46 by atinoco-          #+#    #+#             */
/*   Updated: 2026/08/05 23:19:13 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Compile once. */
static void	run_compile(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (coder->id % 2 == 0)
		take_left_first(coder);
	else
		take_right_first(coder);
	if (!coder_should_continue(coder))
		return ;
	pthread_mutex_lock(&sim->sim_lock);
	coder->last_compile_start = current_time_ms();
	pthread_mutex_unlock(&sim->sim_lock);
	log_state(sim, coder->id, "is compiling");
	update_compile_state(coder, sim);
	sleep_with_stop(sim, sim->config.time_to_compile);
	release_dongles(coder);
}

/* Run the debug step. */
static void	run_debug(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is debugging");
	sleep_with_stop(coder->sim, coder->sim->config.time_to_debug);
}

/* Run the refactor step. */
static void	run_refactor(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is refactoring");
	sleep_with_stop(coder->sim, coder->sim->config.time_to_refactor);
}

/* Main coder loop. */
void	*coder_loop_run(t_coder *coder)
{
	while (coder_should_continue(coder))
	{
		run_compile(coder);
		if (!coder_should_continue(coder))
			break ;
		run_debug(coder);
		if (!coder_should_continue(coder))
			break ;
		run_refactor(coder);
	}
	return (NULL);
}
