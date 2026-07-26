#include "codex.h"

int	dongle_init(t_args *arg, t_simulation *sim)
{
	int	i;

	sim->dongles = NULL;
	sim->dongles = malloc(sizeof(t_dongle) * arg->number_of_coders);
	if (!sim->dongles)
		return (1);
	i = 0;
	while (i < arg->number_of_coders)
	{
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL))
			return (cleanup_dongles(sim, i));
		if (pthread_cond_init(&sim->dongles[i].cond, NULL))
		{
			pthread_mutex_destroy(&sim->dongles[i].lock);
			return (cleanup_dongles(sim, i));
		}
		sim->dongles[i].id = i;
		sim->dongles[i].is_available = 1;
		sim->dongles[i].waiting_count = 0;
		sim->dongles[i].last_released_time = 0;
		sim->dongles[i].waiting_queue.current_n = 0;
		sim->dongles[i].waiting_queue.max_capacity = 2;
		sim->dongles[i].waiting_queue.arr = malloc(sizeof(t_coder *) * 2);
		sim->dongles[i].next_coder = NULL;

		if (!sim->dongles[i].waiting_queue.arr)
			return (1);
		i++;
	}
	return (0);
}

void	request_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->lock);
	// still need the controle over which thread is getting dongle
	
	while (!dongle->is_available || (dongle->next_coder && dongle->next_coder != coder))
	{
		if (!coder->waiting_for_dongle)
		{
			pthread_mutex_lock(&coder->sim->arrival_lock);
			coder->arrival_order = coder->sim->arrival_counter++;
			pthread_mutex_unlock(&coder->sim->arrival_lock);
			heap_push(&dongle->waiting_queue, coder);
			coder->waiting_for_dongle = 1;
		}
		pthread_cond_wait(&dongle->cond, &dongle->lock);
		// printf(" ++ from request_dongle: Coder: %d takes dongle: %d\n", coder->id, dongle->id);
	}
	dongle->is_available = 0;
	dongle->next_coder = NULL;
	
	pthread_mutex_unlock(&dongle->lock);
}


void	release_dongle(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->lock);

	if (!dongle->waiting_queue.current_n)
	{
		dongle->next_coder = NULL;
		dongle->is_available = 1;
	}
	else
	{
		dongle->next_coder = heap_pop(&dongle->waiting_queue);
		dongle->next_coder->waiting_for_dongle = 0;
		dongle->is_available = 1;
	}
	// printf(" -- from release_dongle: Coder: %d release dongle: %d\n", coder->id, dongle->id);
	// if (dongle->next_coder)
	// 	printf("Dongle %d: next coder %d\n",
	// 		dongle->id,
	// 		dongle->next_coder->id);
	// else
	// 	printf("Dongle %d: queue empty\n", dongle->id);
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}


void	connect_dongles(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->args.number_of_coders)
	{
		sim->coders[i].left_dongle = &sim->dongles[i];
		sim->coders[i].right_dongle = &sim->dongles[(i + 1) % sim->args.number_of_coders];
		i++;
	}
}

int	sleep_control(t_simulation *sim, int sleep_time)
{
	long	start;

	start = current_time();
	while (current_time() - start < sleep_time)
	{
		if (sim->simulation_finished)
			return (1);
		usleep(1000);
	}
	return (0);

}

