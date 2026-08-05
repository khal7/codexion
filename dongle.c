#include "codex.h"
int	sim_is_finished(t_simulation *sim);

int	dongle_init(t_args *arg, t_simulation *sim)
{
	int	i;

	sim->dongles = NULL;
	sim->dongles = malloc(sizeof(t_dongle) * arg->number_of_coders);
	sim->args.dongle_cooldown = 0;
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
// int	request_dongle(t_coder *coder, t_dongle *dongle)
// {
// 	pthread_mutex_lock(&dongle->lock);
// 	struct timespec ts;
// 	long deadline_ms;

// 	// Step 1: always register intent first (no more "free = skip the queue")
// 	if (!strcmp(coder->sim->args.scheduler, "fifo"))
// 	{
// 		pthread_mutex_lock(&coder->sim->arrival_lock);
// 		coder->priority = coder->sim->arrival_counter++;
// 		pthread_mutex_unlock(&coder->sim->arrival_lock);
// 	}
// 	else
// 		coder->priority = coder->last_compile_start + coder->sim->args.time_to_burnout;
// 	heap_push(&dongle->waiting_queue, coder);
// 	coder->waiting_for_dongle = 1;

// 	// Step 2: wait until it's actually this coder's turn

// 	while (!sim_is_finished(coder->sim)
// 		&& (!dongle->is_available
// 		|| current_time() - dongle->last_released_time < coder->sim->args.dongle_cooldown
// 		|| dongle->waiting_queue.arr[0] != coder))	
// 	{
// 		pthread_cond_wait(&dongle->cond, &dongle->lock);
// 	}

// 	if (sim_is_finished(coder->sim))
// 	{
// 		pthread_mutex_unlock(&dongle->lock);
// 		return (0);
// 	}

// 	// Step 3: confirmed winner - remove from queue, take the dongle
// 	heap_pop(&dongle->waiting_queue);
// 	coder->waiting_for_dongle = 0;
// 	dongle->is_available = 0;

// 	pthread_mutex_unlock(&dongle->lock);
// 	return (1);
// }

int	request_dongle(t_coder *coder, t_dongle *dongle)
{
	struct timespec ts;
	long remaining;
	long deadline_ms;

	pthread_mutex_lock(&dongle->lock);

	if (!strcmp(coder->sim->args.scheduler, "fifo"))
	{
		pthread_mutex_lock(&coder->sim->arrival_lock);
		coder->priority = coder->sim->arrival_counter++;
		pthread_mutex_unlock(&coder->sim->arrival_lock);
	}
	else
		coder->priority = coder->last_compile_start + coder->sim->args.time_to_burnout;
	heap_push(&dongle->waiting_queue, coder);
	coder->waiting_for_dongle = 1;

	while (!sim_is_finished(coder->sim)
		&& (!dongle->is_available
			|| current_time() - dongle->last_released_time < coder->sim->args.dongle_cooldown
			|| dongle->waiting_queue.arr[0] != coder))
	{
		remaining = coder->sim->args.dongle_cooldown
			- (current_time() - dongle->last_released_time);
		if (remaining < 10)
			remaining = 10;
		deadline_ms = current_time() + remaining;
		ts.tv_sec = deadline_ms / 1000;
		ts.tv_nsec = (deadline_ms % 1000) * 1000000;
		pthread_cond_timedwait(&dongle->cond, &dongle->lock, &ts);
	}

	if (sim_is_finished(coder->sim))
	{
		pthread_mutex_unlock(&dongle->lock);
		return (0);
	}

	heap_pop(&dongle->waiting_queue);
	coder->waiting_for_dongle = 0;
	dongle->is_available = 0;

	pthread_mutex_unlock(&dongle->lock);
	return (1);
}

// void	request_dongle(t_coder *coder, t_dongle *dongle)
// {
// 	pthread_mutex_lock(&dongle->lock);
// 	// still need the controle over which thread is getting dongle
	
// 	while (!dongle->is_available || (dongle->next_coder && dongle->next_coder != coder))
// 	{
		
// 		if (!coder->waiting_for_dongle)
// 		{
// 			if (!strcmp(coder->sim->args.scheduler, "fifo"))
// 			{
// 				pthread_mutex_lock(&coder->sim->arrival_lock);
// 				//printf(".. %d\n", coder->id); 
// 				coder->priority = coder->sim->arrival_counter++;
// 				pthread_mutex_unlock(&coder->sim->arrival_lock);
// 			}
// 			else // edf
// 				coder->priority = coder->last_compile_start + coder->sim->args.time_to_burnout;
// 			heap_push(&dongle->waiting_queue, coder);
// 			coder->waiting_for_dongle = 1;
// 			// pthread_mutex_lock(&coder->sim->arrival_lock);
// 			// coder->arrival_order = coder->sim->arrival_counter++;
// 			// pthread_mutex_unlock(&coder->sim->arrival_lock);
// 			// heap_push(&dongle->waiting_queue, coder);
// 			// coder->waiting_for_dongle = 1;
// 		}
// 		pthread_cond_wait(&dongle->cond, &dongle->lock);
// 		// printf(" ++ from request_dongle: Coder: %d takes dongle: %d\n", coder->id, dongle->id);
// 	}
// 	dongle->is_available = 0;
// 	dongle->next_coder = NULL;
	
// 	pthread_mutex_unlock(&dongle->lock);
// }


// void	release_dongle(t_dongle *dongle, t_coder *coder)
// {
// 	pthread_mutex_lock(&dongle->lock);

// 	if (!dongle->waiting_queue.current_n)
// 	{
// 		dongle->next_coder = NULL;
// 		dongle->is_available = 1;
// 		dongle->last_released_time = current_time();
// 	}
// 	else
// 	{
// 		dongle->next_coder = heap_pop(&dongle->waiting_queue);
// 		dongle->next_coder->waiting_for_dongle = 0;
// 		dongle->is_available = 1;
// 		dongle->last_released_time = current_time();
// 	}
void	release_dongle(t_dongle *dongle, t_coder *coder)
{
	(void)coder;
	pthread_mutex_lock(&dongle->lock);
	dongle->last_released_time = current_time();
	dongle->is_available = 1;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}
	// printf(" -- from release_dongle: Coder: %d release dongle: %d\n", coder->id, dongle->id);
	// if (dongle->next_coder)
	// 	printf("Dongle %d: next coder %d\n",
	// 		dongle->id,
	// 		dongle->next_coder->id);
	// else
	// 	printf("Dongle %d: queue empty\n", dongle->id);
// 	pthread_cond_broadcast(&dongle->cond);
// 	pthread_mutex_unlock(&dongle->lock);
// }


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
		if (sim_is_finished(sim))
			return (1);
		usleep(1000);
	}
	return (0);

}

