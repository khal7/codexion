#include "codex.h"

int	sim_is_finished(t_simulation *sim)
{
	int	val;
	pthread_mutex_lock(&sim->state_lock);
	val = sim->simulation_finished;
	pthread_mutex_unlock(&sim->state_lock);
	return (val);
}

void	sim_set_finished(t_simulation *sim, int value)
{
	pthread_mutex_lock(&sim->state_lock);
	sim->simulation_finished = value;
	pthread_mutex_unlock(&sim->state_lock);
}
// void	*monitor_thread(void *arg)
// {
// 	int	i;
// 	int j;
// 	t_simulation *sim;

// 	i = 0;
// 	sim = (t_simulation *)arg;
// 	while (!sim->simulation_finished)
// 	{
// 		sim->simulation_finished = 1;
// 		if (current_time() - sim->coders[i].last_compile_start >= sim->args.time_to_burnout)
// 		{
// 			sim->simulation_finished = 1;
// 			printing(&sim->coders[i], "burned out");
// 			// exit(1);
// 			break;
// 		}
// 		j = 0;
// 		while (j < sim->args.number_of_coders)
// 		{

// 			if (sim->coders[j].compile_count < sim->args.number_of_compiles_required)
// 			{
// 				sim->simulation_finished = 0;
// 				break;
// 			}	
// 			j++;
// 		}
// 		i++;
// 		if (sim->simulation_finished)
// 			break;
// 		if (i >= sim->args.number_of_coders)
// 			i = 0;
// 		usleep(2000);
// 	}
// 	return (NULL);
// }
void	*monitor_thread(void *arg)
{
	int		i;
	int		j;
	int		all_done;
	long	lcs;
	int		cc;
	t_simulation *sim;

	i = 0;
	sim = (t_simulation *)arg;
	while (!sim_is_finished(sim))
	{
		pthread_mutex_lock(&sim->state_lock);
		lcs = sim->coders[i].last_compile_start;
		cc = sim->coders[i].compile_count;
		pthread_mutex_unlock(&sim->state_lock);

		if (cc < sim->args.number_of_compiles_required
			&& current_time() - lcs >= sim->args.time_to_burnout)
		{
			sim_set_finished(sim, 1);
			printing(&sim->coders[i], "burned out");
			break;
		}

		all_done = 1;
		j = 0;
		while (j < sim->args.number_of_coders)
		{
			pthread_mutex_lock(&sim->state_lock);
			cc = sim->coders[j].compile_count;
			pthread_mutex_unlock(&sim->state_lock);
			if (cc < sim->args.number_of_compiles_required)
			{
				all_done = 0;
				break;
			}
			j++;
		}
		if (all_done)
		{
			sim_set_finished(sim, 1);
			break;
		}
		i++;
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
	char buf[64];

	check = 0;
	if (!request_dongle(coder, first_dongle))
		return (1);   // simulation ended while waiting — give up, don't print anything false
	sprintf(buf, "has taken dongle %d", first_dongle->id);
	printing(coder, buf);

	if (!request_dongle(coder, second_dongle))
	{
		release_dongle(first_dongle, coder);   // give back the first one you already got
		return (1);
	}
	sprintf(buf, "has taken dongle %d", second_dongle->id);
	printing(coder, buf);
	// // printf("Coder %d requesting first dongle\n", coder->id);
	// request_dongle(coder, first_dongle);
	// printing(coder, "has taken a dongle");
	// request_dongle(coder, second_dongle);
	// printing(coder, "has taken a dongle");
	// printf("Coder %d compiling with dongles %d and %d\n",
	// 	coder->id,
	// 	first_dongle->id,
	// 	second_dongle->id);
	pthread_mutex_lock(&coder->sim->state_lock);
	coder->last_compile_start = current_time();
	pthread_mutex_unlock(&coder->sim->state_lock);
	// coder->last_compile_start = current_time();
	printing(coder, "is compiling");
	check = sleep_control(coder->sim, coder->sim->args.time_to_compile);
	if (check)
		return (release_dongle(first_dongle, coder), release_dongle(second_dongle, coder), 1);
	release_dongle(first_dongle, coder);
	release_dongle(second_dongle, coder);
	pthread_mutex_lock(&coder->sim->state_lock);
	coder->compile_count++;
	pthread_mutex_unlock(&coder->sim->state_lock);	
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
	// printf("Coder %d started\n", coder->id);
	while (coder->compile_count < coder->sim->args.number_of_compiles_required && !sim_is_finished(coder->sim))
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
