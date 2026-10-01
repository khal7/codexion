/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khabouou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 16:56:27 by khabouou          #+#    #+#             */
/*   Updated: 2026/08/10 16:56:29 by khabouou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codex.h"

void	*monitor_thread(void *arg)
{
	t_simulation	*sim;

	int (i);
	i = 0;
	sim = (t_simulation *)arg;
	while (!sim_is_finished(sim))
	{
		if (burnout_check(sim, i))
			break ;
		if (all_compile_done(sim))
		{
			sim_set_finished(sim, 1);
			break ;
		}
		i++;
		if (i >= sim->args.number_of_coders)
			i = 0;
		usleep(2000);
	}
	return (NULL);
}

long	current_time(void)
{
	struct timeval	current_time;

	gettimeofday(&current_time, NULL);
	return (current_time.tv_sec * 1000 + current_time.tv_usec / 1000);
}

void	printing(t_coder *coder, char *str)
{
	pthread_mutex_lock(&coder->sim->p_lock);
	printf("%ld %d %s\n", current_time()
		- coder->sim->start_time, coder->id, str);
	pthread_mutex_unlock(&coder->sim->p_lock);
}

int	compile_cycle(t_coder *coder, t_dongle *first_dongle
	, t_dongle *second_dongle)
{
	if (!request_dongle(coder, first_dongle, second_dongle)
		|| sim_is_finished(coder->sim))
		return (1);
	printing_three_lines(coder);
	pthread_mutex_lock(&coder->sim->state_lock);
	coder->last_compile_start = current_time();
	pthread_mutex_unlock(&coder->sim->state_lock);
	if (sleep_control(coder->sim, coder->sim->args.time_to_compile))
		return (release_dongle(first_dongle, coder),
			release_dongle(second_dongle, coder), 1);
	release_dongle(first_dongle, coder);
	release_dongle(second_dongle, coder);
	pthread_mutex_lock(&coder->sim->state_lock);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->sim->state_lock);
	if (sim_is_finished(coder->sim))
		return (1);
	printing(coder, "is debugging");
	if (sleep_control(coder->sim, coder->sim->args.time_to_debug)
		|| sim_is_finished(coder->sim))
		return (1);
	printing(coder, "is refactoring");
	if (sleep_control(coder->sim, coder->sim->args.time_to_refactor))
		return (1);
	return (0);
}

void	*routine(void *arg)
{
	t_coder	*coder;

	int (burnout_check);
	coder = (t_coder *)arg;
	if (coder->id % 2 == 0)
		usleep(1000);
	while (coder->compile_count < coder->sim->args.number_of_compiles_required
		&& !sim_is_finished(coder->sim))
	{
		if (coder->left_dongle->id < coder->right_dongle->id)
		{
			burnout_check = compile_cycle(coder, coder->left_dongle,
					coder->right_dongle);
			if (burnout_check)
				return (NULL);
		}
		else
		{
			burnout_check = compile_cycle(coder, coder->right_dongle,
					coder->left_dongle);
			if (burnout_check)
				return (NULL);
		}
	}
	return (NULL);
}
