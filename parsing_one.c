#include "codex.h"


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


int	thread_creation(t_simulation *sim)
{
	int	i;
	int	j;
	int	error;

	i = 0;
	error = 0;
	while (i < sim->args.number_of_coders)
	{
		if (pthread_create(&sim->coders[i].t, NULL, routine_fun, &sim->coders[i]))
		{
			error = 1;
			break;
		}
		i++;
	}
	j = i;
	i = 0;
	while (i < j)
	{
		if (pthread_join(sim->coders[i].t, NULL))
			error = 1;
		i++;
	}
	return (error);

}

void	*routine_fun(void *arg)
{
	t_coder *coder;

	coder = (t_coder *)arg;
	pthread_mutex_lock(&coder->left_dongle->lock);
	while (!coder->left_dongle->is_available)
		pthread_cond_wait(&coder->left_dongle->cond, &coder->left_dongle->lock);
	coder->left_dongle->is_available = 0;
	pthread_mutex_unlock(&coder->left_dongle->lock);
	return (NULL);
}