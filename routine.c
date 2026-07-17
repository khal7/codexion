#include "codex.h"


void	*monitor_thread(void *arg)
{
	int	i;
	int j;
	t_sim *sim;

	i = 0;
	sim = (t_sim *)arg;
	while (!sim->simulation_finished)
	{
		sim->simulation_finished = 1;
		if (current_time() - sim->coder[i]->last_compile_start >= sim->args->time_to_burnout)
		{
			sim->simulation_finished = 1;
			printing(sim->coder[i], "burned out");
			break;
		}
		j = 0;
		while (j < sim->number_of_coders)
		{
			if (sim->coder[j]->compile_count < sim->args->number_of_compiles_required)
			{
				sim->simulation_finished = 0;
				break;
			}	
			j++;
		}
		i++;
		if (sim->simulation_finished)
			break;
		if (i >= sim->number_of_coders)
			i = 0;
		usleep(2000);
	}
	return (NULL);
}


long	current_time()
{
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	return (current_time.tv_sec * 1000 + current_time.tv_usec / 1000);
}

void	printing(t_coder *coder, char *str)
{
	pthread_mutex_lock(&coder->sim->p_lock);
	printf("%d %d %s\n", coder->last_compile_start, coder->id, str);
	pthread_mutex_unlock(&coder->sim->p_lock);
}

void	compile_cycle(t_coder *coder, t_dongle *first_dongle, t_dongle *second_dongle)
{
	request_dongle(coder, first_dongle);
	printing(coder, "has taken a dongle");
	request_dongle(coder, second_dongle);
	printing(coder, "has taken a dongle");
	coder->last_compile_start = current_time();
	printing(coder, "is compiling");
	usleep(coder->sim->time_to_compile * 1000);
	release_dongle(first_dongle);
	release_dongle(second_dongle);
	coder->compile_count++;
	printing(coder, "is debugging");
	usleep(coder->sim->time_to_debug * 1000);
	printing(coder, "is refactoring");
	usleep(coder->sim->time_to_refactor * 1000);
}

void	*routine(void *arg)
{
	t_coder *coder;

	coder = (t_coder *)arg;
	while (coder->compile_count < coder->sim->number_of_compiles_required)
	{

		if (coder->left_dongle->id < coder->right_dongle->id)
			compile_cycle(coder, coder->left_dongle, coder->right_dongle);
		else
			compile_cycle(coder, coder->right_dongle, coder->left_dongle);
	}
	return (NULL);
	while (!coder->left_dongle->is_available)
		pthread_cond_wait(&coder->left_dongle->cond, &coder->left_dongle->lock);
	coder->left_dongle->is_available = 0;
	pthread_mutex_unlock(&coder->left_dongle->lock);
}
