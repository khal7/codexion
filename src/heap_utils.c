/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khabouou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 16:55:48 by khabouou          #+#    #+#             */
/*   Updated: 2026/08/10 16:55:55 by khabouou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codex.h"

void	ft_swap(t_coder **a, t_coder **b)
{
	t_coder	*tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heapify_up(t_heap *heap, int index)
{
	while (index)
	{
		if (heap->arr[index]->priority < heap->arr[(index - 1) / 2]->priority
			|| (heap->arr[index]->priority
				== heap->arr[(index - 1) / 2]->priority
				&& heap->arr[index]->id < heap->arr[(index - 1) / 2]->id))
		{
			ft_swap(&heap->arr[index], &heap->arr[(index - 1) / 2]);
			index = (index - 1) / 2;
		}
		else
			break ;
	}
}

void	heap_push(t_heap *heap, t_coder *coder)
{
	if (heap->current_n >= heap->max_capacity)
		return ;
	heap->arr[heap->current_n] = coder;
	heap->current_n++;
	heapify_up(heap, heap->current_n - 1);
}

void	heapify_down(t_heap *heap, int index)
{
	int	left;
	int	right;
	int	small;

	while (1)
	{
		left = 2 * index + 1;
		right = 2 * index + 2;
		if (left >= heap->current_n)
			break ;
		small = left;
		if (right < heap->current_n
			&& (heap->arr[right]->priority < heap->arr[left]->priority
				|| (heap->arr[right]->priority == heap->arr[left]->priority
					&& heap->arr[right]->id < heap->arr[left]->id)))
			small = right;
		if (heap->arr[index]->priority < heap->arr[small]->priority
			|| (heap->arr[index]->priority == heap->arr[small]->priority
				&& heap->arr[index]->id <= heap->arr[small]->id))
			break ;
		ft_swap(&heap->arr[index], &heap->arr[small]);
		index = small;
	}
}

t_coder	*heap_pop(t_heap *heap)
{
	t_coder	*res;

	if (heap->current_n == 0)
		return (NULL);
	res = heap->arr[0];
	heap->current_n--;
	heap->arr[0] = heap->arr[heap->current_n];
	heapify_down(heap, 0);
	return (res);
}
