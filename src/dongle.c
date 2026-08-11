/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khabouou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 16:54:23 by khabouou          #+#    #+#             */
/*   Updated: 2026/08/10 16:54:29 by khabouou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codex.h"

int	dongle_init(t_args *arg, t_simulation *sim)
{
	int (i);
	sim->dongles = NULL;
	sim->dongles = malloc(sizeof(t_dongle) * arg->number_of_coders);
	sim->start_time = 0;
	sim->simulation_finished = 0;
	sim->arrival_counter = 0;
	pthread_mutex_init(&sim->queue_lock, NULL);
	pthread_mutex_init(&sim->arrival_lock, NULL);
	pthread_mutex_init(&sim->p_lock, NULL);
	pthread_mutex_init(&sim->state_lock, NULL);
	if (!sim->dongles)
		return (1);
	i = 0;
	while (i < arg->number_of_coders)
	{
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL))
			return (cleanup_dongles(sim, i));
		sim->dongles[i].id = i;
		sim->dongles[i].is_available = 1;
		sim->dongles[i].waiting_count = 0;
		sim->dongles[i].last_released_time = 0;
		sim->dongles[i].next_coder = NULL;
		i++;
	}
	return (0);
}

void	release_dongle(t_dongle *dongle, t_coder *coder)
{
	(void)coder;
	pthread_mutex_lock(&dongle->lock);
	dongle->last_released_time = current_time();
	dongle->is_available = 1;
	pthread_mutex_unlock(&dongle->lock);
}

void	connect_dongles(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		sim->coders[i].left_dongle = &sim->dongles[i];
		sim->coders[i].right_dongle = &sim->dongles[(i + 1)
			% sim->args.number_of_coders];
		i++;
	}
}

int	sleep_control(t_simulation *sim, int sleep_time)
{
	long	start;

	start = current_time();
	while (current_time() - start < sleep_time)
	{
		if (sim_is_finished(sim))
			return (1);
		usleep(1000);
	}
	return (0);
}
