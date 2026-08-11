/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khabouou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 16:56:33 by khabouou          #+#    #+#             */
/*   Updated: 2026/08/10 16:56:35 by khabouou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codex.h"

int	sim_is_finished(t_simulation *sim)
{
	int	val;

	pthread_mutex_lock(&sim->state_lock);
	val = sim->simulation_finished;
	pthread_mutex_unlock(&sim->state_lock);
	return (val);
}

void	sim_set_finished(t_simulation *sim, int value)
{
	pthread_mutex_lock(&sim->state_lock);
	sim->simulation_finished = value;
	pthread_mutex_unlock(&sim->state_lock);
}

int	one_coder(t_simulation *sim, t_coder *coder, t_dongle *first_dongle)
{
	pthread_mutex_lock(&first_dongle->lock);
	if (first_dongle->is_available)
	{
		first_dongle->is_available = 0;
		pthread_mutex_unlock(&first_dongle->lock);
		printing(coder, "has taken a dongle");
	}
	else
		pthread_mutex_unlock(&first_dongle->lock);
	while (!sim_is_finished(sim))
		usleep(1000);
	return (0);
}

int	burnout_check(t_simulation *sim, int i)
{
	int		cc;
	long	lcs;

	pthread_mutex_lock(&sim->state_lock);
	lcs = sim->coders[i].last_compile_start;
	cc = sim->coders[i].compile_count;
	pthread_mutex_unlock(&sim->state_lock);
	if (cc < sim->args.number_of_compiles_required
		&& current_time() - lcs >= sim->args.time_to_burnout)
	{
		sim_set_finished(sim, 1);
		printing(&sim->coders[i], "burned out");
		return (1);
	}
	return (0);
}

int	all_compile_done(t_simulation *sim)
{
	int	j;
	int	cc;

	j = 0;
	while (j < sim->args.number_of_coders)
	{
		pthread_mutex_lock(&sim->state_lock);
		cc = sim->coders[j].compile_count;
		pthread_mutex_unlock(&sim->state_lock);
		if (cc < sim->args.number_of_compiles_required)
			return (0);
		j++;
	}
	return (1);
}
