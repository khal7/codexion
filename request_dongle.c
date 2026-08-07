
#include "codex.h"
int	sim_is_finished(t_simulation *sim);

t_dongle	*lock_order(t_dongle *first_dongle, t_dongle *second_dongle)
{
	t_dongle	*low;
	t_dongle	*high;

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
	pthread_mutex_lock(&low->lock);
	pthread_mutex_lock(&high->lock);
	return (low);
}

void	register_coder(t_coder *coder, t_dongle *low, t_dongle *high)
{
	t_simulation	*sim;

	sim = coder->sim;
	if (coder->waiting_for_dongle)
        return;
	if (!strcmp(sim->args.scheduler, "fifo"))
	{
		pthread_mutex_lock(&sim->arrival_lock);
		coder->priority = sim->arrival_counter++;
		pthread_mutex_unlock(&sim->arrival_lock);
	}
	else
		coder->priority = coder->last_compile_start + sim->args.time_to_burnout;
	heap_push(&low->waiting_queue, coder);
	heap_push(&high->waiting_queue, coder);
	coder->waiting_for_dongle = 1;
}

int	dongle_ready(t_dongle *d, t_coder *coder, long cooldown)
{
	if (d->waiting_queue.current_n == 0)
    return (0);

	if (d->waiting_queue.arr[0] != coder)
		return (0);
	if (!d->is_available)
		return (0);
	if (current_time() - d->last_released_time < cooldown)
		return (0);
	if (d->waiting_queue.arr[0] != coder)
		return (0);
	return (1);
}

int	both_ready(t_coder *coder, t_dongle *low, t_dongle *high)
{
	long	cooldown;

	cooldown = coder->sim->args.dongle_cooldown;
	if (!dongle_ready(low, coder, cooldown))
		return (0);
	if (!dongle_ready(high, coder, cooldown))
		return (0);
	return (1);
}

void	wait_for_dongles(t_coder *coder, t_dongle *low, t_dongle *high)
{
	struct timespec	ts;
	long			remaining_low;
	long			remaining_high;
	long			remaining;
	long			deadline_ms;
	long			cooldown;

	cooldown = coder->sim->args.dongle_cooldown;
	remaining_low = cooldown - (current_time() - low->last_released_time);
	remaining_high = cooldown - (current_time() - high->last_released_time);
	remaining = remaining_low < remaining_high ? remaining_low : remaining_high;
	if (remaining < 10)
		remaining = 10;
	deadline_ms = current_time() + remaining;
	ts.tv_sec = deadline_ms / 1000;
	ts.tv_nsec = (deadline_ms % 1000) * 1000000;
	pthread_mutex_unlock(&high->lock);
	pthread_cond_timedwait(&coder->sim->dongle_cond, &low->lock, &ts);
	pthread_mutex_lock(&high->lock);
}

void	take_dongles(t_coder *coder, t_dongle *low, t_dongle *high)
{
	if (low->waiting_queue.arr[0] != coder)
    	return;

	if (high->waiting_queue.arr[0] != coder)
		return;
	heap_pop(&low->waiting_queue);
	heap_pop(&high->waiting_queue);
	coder->waiting_for_dongle = 0;
	low->is_available = 0;
	high->is_available = 0;
}


int	request_dongle(t_coder *coder, t_dongle *first_dongle, t_dongle *second_dongle)
{
	t_dongle	*low;
	t_dongle	*high;

	low = lock_order(first_dongle, second_dongle);
	if (low == first_dongle)
		high = second_dongle;
	else
		high = first_dongle;
	register_coder(coder, low, high);
	pthread_mutex_unlock(&high->lock);
	pthread_mutex_unlock(&low->lock);
	while (!sim_is_finished(coder->sim))
	{
		pthread_mutex_lock(&low->lock);
		pthread_mutex_lock(&high->lock);
		if (both_ready(coder, low, high))
		{
			take_dongles(coder, low, high);
			pthread_mutex_unlock(&high->lock);
			pthread_mutex_unlock(&low->lock);
			return (1);
		}
		pthread_mutex_unlock(&high->lock);
		pthread_mutex_unlock(&low->lock);
		//sleep_control(coder->sim, 100);
		usleep(100);
	}
	return (0);
}
