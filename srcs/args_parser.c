/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   args_parser.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/22 01:32:28 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/23 12:33:56 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Convert a numeric string to a positive long long. */
static int	parse_positive_llong(char *str, long long *out)
{
	long long	result;

	if (!str || !str[0])
		return (0);
	if (str[0] == '+')
		str++;
	if (!str[0])
		return (0);
	result = 0;
	while (*str)
	{
		if (*str < '0' || *str > '9')
			return (0);
		if (result > (LLONG_MAX - (*str - '0')) / 10)
			return (0);
		result = result * 10 + (*str - '0');
		str++;
	}
	*out = result;
	return (1);
}

/* Set scheduler mode from the "fifo"/"edf" string argument. */
static int	set_scheduler(t_sim *sim, char *str)
{
	if (strcmp(str, "fifo") == 0)
		sim->config.scheduler = SCHED_POLICY_FIFO;
	else if (strcmp(str, "edf") == 0)
		sim->config.scheduler = SCHED_POLICY_EDF;
	else
		return (printf("Error: scheduler must be fifo or edf\n"), 0);
	return (1);
}

/* Map argument index i (1..7) to its config field. */
static void	assign_field(t_sim *sim, int i, long long value)
{
	if (i == 1)
		sim->config.num_coders = (int)value;
	else if (i == 2)
		sim->config.time_to_burnout = value;
	else if (i == 3)
		sim->config.time_to_compile = value;
	else if (i == 4)
		sim->config.time_to_debug = value;
	else if (i == 5)
		sim->config.time_to_refactor = value;
	else if (i == 6)
		sim->config.compiles_required = (int)value;
	else
		sim->config.dongle_cooldown = value;
}

/* Validate argc, then parse all arguments into sim->config. */
int	parse_args(t_sim *sim, int argc, char **argv)
{
	long long	value;
	int			i;

	if (argc != 9)
		return (printf("Error: expected 8 arguments\n"), 0);
	i = 1;
	while (i <= 7)
	{
		if (!parse_positive_llong(argv[i], &value))
			return (printf("Error: argument %d invalid\n", i), 0);
		if (value == 0 && i != 7)
			return (printf("Error: argument %d must be > 0\n", i), 0);
		if ((i == 1 || i == 6) && value > INT_MAX)
			return (printf("Error: argument %d is too large\n", i), 0);
		assign_field(sim, i, value);
		i++;
	}
	return (set_scheduler(sim, argv[8]));
}
