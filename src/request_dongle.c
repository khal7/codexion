
#include "codex.h"
int	sim_is_finished(t_simulation *sim);

// t_dongle	*lock_order(t_dongle *first_dongle, t_dongle *second_dongle)
// {
// 	t_dongle	*low;
// 	t_dongle	*high;

// 	if (first_dongle->id < second_dongle->id)
// 	{
// 		low = first_dongle;
// 		high = second_dongle;
// 	}
// 	else
// 	{
// 		low = second_dongle;
// 		high = first_dongle;
// 	}
// 	pthread_mutex_lock(&low->lock);
// 	pthread_mutex_lock(&high->lock);
// 	return (low);
// }

// void	register_coder(t_coder *coder, t_dongle *low, t_dongle *high)
// {
// 	t_simulation	*sim;

// 	sim = coder->sim;
// 	if (coder->waiting_for_dongle)
//         return;
// 	if (!strcmp(sim->args.scheduler, "fifo"))
// 	{
// 		pthread_mutex_lock(&sim->arrival_lock);
// 		coder->priority = sim->arrival_counter++;
// 		pthread_mutex_unlock(&sim->arrival_lock);
// 	}
// 	else
// 		coder->priority = coder->last_compile_start + sim->args.time_to_burnout;
// 	heap_push(&low->waiting_queue, coder);
// 	heap_push(&high->waiting_queue, coder);
// 	coder->waiting_for_dongle = 1;
// }

// int	dongle_ready(t_dongle *d, t_coder *coder, long cooldown)
// {
// 	if (d->waiting_queue.current_n == 0)
//     return (0);

// 	if (d->waiting_queue.arr[0] != coder)
// 		return (0);
// 	if (!d->is_available)
// 		return (0);
// 	if (current_time() - d->last_released_time < cooldown)
// 		return (0);
// 	if (d->waiting_queue.arr[0] != coder)
// 		return (0);
// 	return (1);
// }

// int	both_ready(t_coder *coder, t_dongle *low, t_dongle *high)
// {
// 	long	cooldown;

// 	cooldown = coder->sim->args.dongle_cooldown;
// 	if (!dongle_ready(low, coder, cooldown))
// 		return (0);
// 	if (!dongle_ready(high, coder, cooldown))
// 		return (0);
// 	return (1);
// }

// void	wait_for_dongles(t_coder *coder, t_dongle *low, t_dongle *high)
// {
// 	struct timespec	ts;
// 	long			remaining_low;
// 	long			remaining_high;
// 	long			remaining;
// 	long			deadline_ms;
// 	long			cooldown;

// 	cooldown = coder->sim->args.dongle_cooldown;
// 	remaining_low = cooldown - (current_time() - low->last_released_time);
// 	remaining_high = cooldown - (current_time() - high->last_released_time);
// 	remaining = remaining_low < remaining_high ? remaining_low : remaining_high;
// 	if (remaining < 10)
// 		remaining = 10;
// 	deadline_ms = current_time() + remaining;
// 	ts.tv_sec = deadline_ms / 1000;
// 	ts.tv_nsec = (deadline_ms % 1000) * 1000000;
// 	pthread_mutex_unlock(&high->lock);
// 	pthread_cond_timedwait(&coder->sim->dongle_cond, &low->lock, &ts);
// 	pthread_mutex_lock(&high->lock);
// }

// void	take_dongles(t_coder *coder, t_dongle *low, t_dongle *high)
// {
// 	if (low->waiting_queue.arr[0] != coder)
//     	return;

// 	if (high->waiting_queue.arr[0] != coder)
// 		return;
// 	heap_pop(&low->waiting_queue);
// 	heap_pop(&high->waiting_queue);
// 	coder->waiting_for_dongle = 0;
// 	low->is_available = 0;
// 	high->is_available = 0;
// }

// int	request_dongle(t_coder *coder, t_dongle *first_dongle, t_dongle *second_dongle)
// {
// 	t_dongle	*low;
// 	t_dongle	*high;

// 	low = lock_order(first_dongle, second_dongle);
// 	if (low == first_dongle)
// 		high = second_dongle;
// 	else
// 		high = first_dongle;
// 	register_coder(coder, low, high);
// 	pthread_mutex_unlock(&high->lock);
// 	pthread_mutex_unlock(&low->lock);
// 	while (!sim_is_finished(coder->sim))
// 	{
// 		pthread_mutex_lock(&low->lock);
// 		pthread_mutex_lock(&high->lock);
// 		if (both_ready(coder, low, high))
// 		{
// 			take_dongles(coder, low, high);
// 			pthread_mutex_unlock(&high->lock);
// 			pthread_mutex_unlock(&low->lock);
// 			return (1);
// 		}
// 		pthread_mutex_unlock(&high->lock);
// 		pthread_mutex_unlock(&low->lock);
// 		//sleep_control(coder->sim, 100);
// 		usleep(100);
// 	}
// 	return (0);
// }
int	is_higher_priority(t_coder *a, t_coder *b)
{
	if (a->priority != b->priority)
		return (a->priority < b->priority);
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

int	allowed_to_take(t_simulation *sim, t_coder *coder, t_dongle *low, t_dongle *high)
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
	pthread_mutex_lock(&sim->queue_lock);
	heap_push(&sim->global_queue, coder);
	pthread_mutex_unlock(&sim->queue_lock);
	coder->waiting_for_dongle = 1;
}
int	try_take_dongles(t_coder *coder, t_dongle *low, t_dongle *high)
{
	long	cooldown;
	int		ok;

	(void)coder;
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

int	try_acquire(t_simulation *sim, t_coder *coder, t_dongle *low, t_dongle *high)
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

int	request_dongle(t_coder *coder, t_dongle *first_dongle, t_dongle *second_dongle)
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
	abandon_request(sim, coder);
	return (0);
}
