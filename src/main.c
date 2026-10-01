/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khabouou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 16:55:58 by khabouou          #+#    #+#             */
/*   Updated: 2026/08/10 16:56:01 by khabouou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codex.h"

void	printing_three_lines(t_coder *coder)
{
	pthread_mutex_lock(&coder->sim->p_lock);
	printf("%ld %d %s\n", current_time()
		- coder->sim->start_time, coder->id, "has taken a dongle");
	printf("%ld %d %s\n", current_time()
		- coder->sim->start_time, coder->id, "has taken a dongle");
	printf("%ld %d %s\n", current_time()
		- coder->sim->start_time, coder->id, "is compiling");
	pthread_mutex_unlock(&coder->sim->p_lock);
}

void	clean_simu(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		pthread_mutex_destroy(&sim->dongles[i].lock);
		i++;
	}
	pthread_mutex_destroy(&sim->queue_lock);
	pthread_mutex_destroy(&sim->p_lock);
	pthread_mutex_destroy(&sim->arrival_lock);
	pthread_mutex_destroy(&sim->state_lock);
	free(sim->global_queue.arr);
	free(sim->dongles);
	free(sim->coders);
}

int	main(int ac, char **av)
{
	t_simulation	sim;
	t_args			arg;

	if (ac != 9)
	{
		printf("The number of argument should be 8.\n");
		return (0);
	}
	if (args_to_struct(ac, av, &arg) == -1)
	{
		printf("Invalid arguments.\n");
		return (0);
	}
	if (dongle_init(&arg, &sim) || coder_init(&arg, &sim))
		return (1);
	thread_creation(&sim);
	clean_simu(&sim);
	return (0);
}
