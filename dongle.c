

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
		sim->dongles[i].is_available = 1;
		sim->dongles[i].waiting_count = 0;
		sim->dongles[i].last_released_time = 0;
		i++;
	}
	return (0);
}



void	request_dongle(t_coder *coder, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->lock);

	while (!dongle->is_available)
		pthread_cond_wait(&dongle->cond, &dongle->lock);

	dongle->is_available = 0;
	pthread_mutex_unlock(&dongle->lock);
}


void	release_dongle(t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->lock);

	dongle->is_available = 1;
	pthread_cond_signal(&dongle->cond);
	pthread_mutex_unlock(&dongle->lock);
}