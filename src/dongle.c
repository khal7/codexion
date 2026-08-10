#include "codex.h"
int	sim_is_finished(t_simulation *sim);

int	dongle_init(t_args *arg, t_simulation *sim)
{
	int	i;

	sim->dongles = NULL;
	sim->dongles = malloc(sizeof(t_dongle) * arg->number_of_coders);
	//sim->args.dongle_cooldown = 0;
	pthread_mutex_init(&sim->queue_lock, NULL);
	if (!sim->dongles)
		return (1);
	i = 0;
	while (i < arg->number_of_coders)
	{
		if (pthread_mutex_init(&sim->dongles[i].lock, NULL))
			return (cleanup_dongles(sim, i));
		// if (pthread_cond_init(&sim->dongles[i].cond, NULL))
		// {
		// 	pthread_mutex_destroy(&sim->dongles[i].lock);
		// 	return (cleanup_dongles(sim, i));
		// }
		sim->dongles[i].id = i;
		sim->dongles[i].is_available = 1;
		sim->dongles[i].waiting_count = 0;
		sim->dongles[i].last_released_time = 0;
		//sim->dongles[i].waiting_queue.current_n = 0;
		//sim->dongles[i].waiting_queue.max_capacity = 2;
		//sim->dongles[i].waiting_queue.arr = malloc(sizeof(t_coder *) * 2);
		sim->dongles[i].next_coder = NULL;


		// if (!sim->dongles[i].waiting_queue.arr)
		// 	return (1);
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
	//pthread_cond_broadcast(&dongle->cond);
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
		if (sim_is_finished(sim))
			return (1);
		usleep(1000);
	}
	return (0);

}

