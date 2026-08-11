/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request_dongle.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khabouou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 16:56:14 by khabouou          #+#    #+#             */
/*   Updated: 2026/08/10 16:56:15 by khabouou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codex.h"

int	heap_find(t_heap *heap, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < heap->current_n)
	{
		if (heap->arr[i] == coder)
			return (i);
		i++;
	}
	return (-1);
}

void	heap_remove_at(t_heap *heap, int index)
{
	if (index < 0 || index >= heap->current_n)
		return ;
	heap->arr[index] = heap->arr[heap->current_n - 1];
	heap->current_n--;
	heapify_down(heap, index);
	heapify_up(heap, index);
}

int	try_acquire(t_simulation *sim, t_coder *coder,
	t_dongle *low, t_dongle *high)
{
	int	allowed;
	int	idx;

	pthread_mutex_lock(&sim->queue_lock);
	allowed = allowed_to_take(sim, coder, low, high);
	pthread_mutex_unlock(&sim->queue_lock);
	if (allowed && try_take_dongles(coder, low, high))
	{
		pthread_mutex_lock(&sim->queue_lock);
		idx = heap_find(&sim->global_queue, coder);
		heap_remove_at(&sim->global_queue, idx);
		pthread_mutex_unlock(&sim->queue_lock);
		coder->waiting_for_dongle = 0;
		return (1);
	}
	return (0);
}

void	abandon_request(t_simulation *sim, t_coder *coder)
{
	int	idx;

	pthread_mutex_lock(&sim->queue_lock);
	idx = heap_find(&sim->global_queue, coder);
	if (idx != -1)
		heap_remove_at(&sim->global_queue, idx);
	pthread_mutex_unlock(&sim->queue_lock);
	coder->waiting_for_dongle = 0;
}

int	request_dongle(t_coder *coder, t_dongle *first_dongle,
	t_dongle *second_dongle)
{
	t_simulation	*sim;
	t_dongle		*low;
	t_dongle		*high;

	sim = coder->sim;
	if (sim->args.number_of_coders == 1)
		return (one_coder(sim, coder, first_dongle));
	if (first_dongle->id < second_dongle->id)
	{
		low = first_dongle;
		high = second_dongle;
	}
	else
	{
		low = second_dongle;
		high = first_dongle;
	}
	register_coder(coder, sim);
	while (!sim_is_finished(sim))
	{
		if (try_acquire(sim, coder, low, high))
			return (1);
		usleep(100);
	}
	return (abandon_request(sim, coder), 0);
}
