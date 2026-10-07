/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khabouou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 16:56:06 by khabouou          #+#    #+#             */
/*   Updated: 2026/08/10 16:56:10 by khabouou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codex.h"

long	ft_atoi(char *str)
{
	long	result;
	int		digit;

	result = 0;
	if (*str == '+')
		str++;
	if (*str == '\0')
		return (-1);
	while (*str)
	{
		if (*str < '0' || *str > '9')
			return (-1);
		digit = *str - '0';
		if (result > LONG_MAX / 10
			|| (result == LONG_MAX / 10 && digit > LONG_MAX % 10))
			return (-1);
		result = result * 10 + digit;
		str++;
	}
	return (result);
}

int	args_to_struct(int ac, char **av, t_args *ptr)
{
	int	i;

	i = 1;
	while (av[i] && i < (ac - 1))
	{
		if (ft_atoi(av[i]) == -1)
			return (-1);
		i++;
	}
	if (strcmp(av[8], "fifo") && strcmp(av[8], "edf"))
		return (-1);
	ptr->number_of_coders = ft_atoi(av[1]);
	ptr->time_to_burnout = ft_atoi(av[2]);
	ptr->time_to_compile = ft_atoi(av[3]);
	ptr->time_to_debug = ft_atoi(av[4]);
	ptr->time_to_refactor = ft_atoi(av[5]);
	ptr->number_of_compiles_required = ft_atoi(av[6]);
	ptr->dongle_cooldown = ft_atoi(av[7]);
	ptr->scheduler = av[8];
	return (0);
}

int	coder_init(t_args *arg, t_simulation *sim)
{
	int	i;

	sim->args = *arg;
	sim->coders = NULL;
	sim->coders = malloc(sizeof(t_coder) * arg->number_of_coders);
	if (!sim->coders)
		return (cleanup_dongles(sim), 1);
	sim->global_queue.current_n = 0;
	sim->global_queue.max_capacity = arg->number_of_coders;
	sim->global_queue.arr = malloc(sizeof(t_coder *) * arg->number_of_coders);
	if (!sim->global_queue.arr)
		return (1);
	i = 0;
	while (i < arg->number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].last_compile_start = 0;
		sim->coders[i].compile_count = 0;
		sim->coders[i].burned_out = 0;
		sim->coders[i].sim = sim;
		sim->coders[i].waiting_for_dongle = 0;
		sim->coders[i].priority = 0;
		i++;
	}
	return (connect_dongles(sim), 0);
}

int	cleanup_dongles(t_simulation *sim)
{
	free(sim->dongles);
	sim->dongles = NULL;
	return (1);
}

int	thread_creation(t_simulation *sim)
{
	int (i), (j), (error);
	i = 0;
	error = 0;
	sim->start_time = current_time();
	while (i < sim->args.number_of_coders)
		sim->coders[i++].last_compile_start = sim->start_time;
	i = 0;
	while (i < sim->args.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].t, NULL, routine, &sim->coders[i]))
		{
			error = 1;
			break ;
		}
		i++;
	}
	j = i;
	i = 0;
	pthread_create(&sim->monitor, NULL, monitor_thread, sim);
	while (i < j)
		if (pthread_join(sim->coders[i++].t, NULL))
			error = 1;
	return (pthread_join(sim->monitor, NULL), error);
}
