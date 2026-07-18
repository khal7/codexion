#include "codex.h"


void	*monitor_thread(void *arg)
{
	int	i;
	int j;
	t_simulation *sim;

	i = 0;
	sim = (t_simulation *)arg;
	while (!sim->simulation_finished)
	{
		sim->simulation_finished = 1;
		if (current_time() - sim->coders[i].last_compile_start >= sim->args.time_to_burnout)
		{
			sim->simulation_finished = 1;
			printing(&sim->coders[i], "burned out");
			break;
		}
		j = 0;
		while (j < sim->args.number_of_coders)
		{
			if (sim->coders[j].compile_count < sim->args.number_of_compiles_required)
			{
				sim->simulation_finished = 0;
				break;
			}	
			j++;
		}
		i++;
		if (sim->simulation_finished)
			break;
		if (i >= sim->args.number_of_coders)
			i = 0;
		usleep(2000);
	}
	return (NULL);
}


long	current_time(void)
{
	struct timeval current_time;
	gettimeofday(&current_time, NULL);
	return (current_time.tv_sec * 1000 + current_time.tv_usec / 1000);
}

void	printing(t_coder *coder, char *str)
{
	pthread_mutex_lock(&coder->sim->p_lock);
	printf("%ld %d %s\n", current_time() - coder->sim->start_time, coder->id, str);
	pthread_mutex_unlock(&coder->sim->p_lock);
}

int	compile_cycle(t_coder *coder, t_dongle *first_dongle, t_dongle *second_dongle)
{
	int check;

	check = 0;
	request_dongle(coder, first_dongle);
	printing(coder, "has taken a dongle");
	request_dongle(coder, second_dongle);
	printing(coder, "has taken a dongle");
	coder->last_compile_start = current_time();
	printing(coder, "is compiling");
	check = sleep_control(coder->sim, coder->sim->args.time_to_compile);
	if (check)
		return (release_dongle(first_dongle), release_dongle(second_dongle), 1);
	release_dongle(first_dongle);
	release_dongle(second_dongle);
	coder->compile_count++;
	printing(coder, "is debugging");
	check = sleep_control(coder->sim, coder->sim->args.time_to_debug );
	if (check)
		return (1);
	printing(coder, "is refactoring");
	check = sleep_control(coder->sim, coder->sim->args.time_to_refactor);
	if (check)
		return (1);
	return (0);
}

void	*routine(void *arg)
{
	t_coder *coder;
	int	burnout_check;

	burnout_check = 0;
	coder = (t_coder *)arg;
	while (coder->compile_count < coder->sim->args.number_of_compiles_required && !coder->sim->simulation_finished)
	{

		if (coder->left_dongle->id < coder->right_dongle->id)
		{
			burnout_check = compile_cycle(coder, coder->left_dongle, coder->right_dongle);
			if (burnout_check)
				return (NULL);
		}	
		else
		{
			burnout_check = compile_cycle(coder, coder->right_dongle, coder->left_dongle);
			if (burnout_check)
			return (NULL);
		}
	}
	return (NULL);
}
