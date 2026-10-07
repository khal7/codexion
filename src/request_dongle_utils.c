/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_dongle_utils.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khabouou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 16:56:19 by khabouou          #+#    #+#             */
/*   Updated: 2026/08/10 16:56:21 by khabouou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codex.h"

int	is_higher_priority(t_coder *a, t_coder *b)
{
	if (a->priority != b->priority)
		return (a->priority < b->priority);
	if (a->queue_arrival_time != b->queue_arrival_time)
		return (a->queue_arrival_time < b->queue_arrival_time);
	return (a->id < b->id);
}

int	shares_dongle(t_coder *a, t_dongle *low, t_dongle *high)
{
	if (a->left_dongle == low || a->left_dongle == high)
		return (1);
	if (a->right_dongle == low || a->right_dongle == high)
		return (1);
	return (0);
}

int	allowed_to_take(t_simulation *sim, t_coder *coder,
	t_dongle *low, t_dongle *high)
{
	int		i;
	t_coder	*other;

	i = 0;
	while (i < sim->global_queue.current_n)
	{
		other = sim->global_queue.arr[i];
		if (other != coder && shares_dongle(other, low, high)
			&& is_higher_priority(other, coder))
			return (0);
		i++;
	}
	return (1);
}

void	register_coder(t_coder *coder, t_simulation *sim)
{
	if (coder->waiting_for_dongle)
		return ;
	if (!strcmp(sim->args.scheduler, "fifo"))
	{
		pthread_mutex_lock(&sim->arrival_lock);
		coder->priority = sim->arrival_counter++;
		pthread_mutex_unlock(&sim->arrival_lock);
	}
	else
		coder->priority = coder->last_compile_start + sim->args.time_to_burnout;
	coder->queue_arrival_time = current_time();
	pthread_mutex_lock(&sim->queue_lock);
	heap_push(&sim->global_queue, coder);
	pthread_mutex_unlock(&sim->queue_lock);
	coder->waiting_for_dongle = 1;
}

int	try_take_dongles(t_coder *coder, t_dongle *low, t_dongle *high)
{
	long	cooldown;
	int		ok;

	cooldown = coder->sim->args.dongle_cooldown;
	pthread_mutex_lock(&low->lock);
	pthread_mutex_lock(&high->lock);
	ok = low->is_available && high->is_available
		&& current_time() - low->last_released_time >= cooldown
		&& current_time() - high->last_released_time >= cooldown;
	if (ok)
	{
		low->is_available = 0;
		high->is_available = 0;
	}
	pthread_mutex_unlock(&high->lock);
	pthread_mutex_unlock(&low->lock);
	return (ok);
}
