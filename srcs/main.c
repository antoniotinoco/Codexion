/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 00:21:08 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/21 00:21:08 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdlib.h>
#include <stdio.h>

int	main(int argc, char **argv)
{
	t_sim	*sim;

	sim = malloc(sizeof(t_sim));
	if (!sim)
		return (printf("Error: memory allocation failed\n"), 1);
	if (!setup_simulation(sim, argc, argv))
		return (free(sim), 1);
	if (!thread_manager_start(sim))
		return (thread_manager_cleanup(sim), free(sim), 1);
	if (!thread_manager_join(sim))
		return (thread_manager_cleanup(sim), free(sim), 1);
	thread_manager_cleanup(sim);
	free(sim);
	return (0);
}
