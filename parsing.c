
#include "codex.h"


long	ft_atoi(char *str)
{
	long	result;
	int		digit;

	result = 0;

	if (*str == '+')
		str++;
	if (*str == '\0')
		return (-1);
	while (*str)
	{
		if (*str < '0' || *str > '9')
			return (-1);
		digit = *str - '0';
		if (result > LONG_MAX / 10
			|| (result == LONG_MAX / 10 && digit > LONG_MAX % 10))
			return (-1);
		result = result * 10 + digit;
		str++;
	}
	return (result);
}

int  args_to_struct(int ac, char **av, t_args *ptr)
{
	int i;
	int j;	
	i = 1;
	while (av[i] && i < (ac - 1))
	{
		if (ft_atoi(av[i]) == -1)
			return (-1);
		i++;
	}
	if (strcmp(av[8], "fifo") && strcmp(av[8], "edf")) 
		return (-1);
	ptr->number_of_coders = ft_atoi(av[1]);
	ptr->time_to_burnout = ft_atoi(av[2]);
	ptr->time_to_compile = ft_atoi(av[3]);
	ptr->time_to_debug = ft_atoi(av[4]);
	ptr->time_to_refactor = ft_atoi(av[5]);
	ptr->number_of_compiles_required = ft_atoi(av[6]);
	ptr->dongle_cooldown = ft_atoi(av[7]);
	ptr->scheduler = av[8];
	return (0);
}

int coder_init(t_args *arg, t_simulation *sim)
{
	int	i;
	
	sim->args = *arg;
	sim->coders = NULL;
	sim->coders = malloc(sizeof(t_coder) * arg->number_of_coders);
	if (!sim->coders)
		return (cleanup_dongles(sim, sim->args.number_of_coders), 1);
	i = 0;
	while (i < arg->number_of_coders)
	{
		sim->coders[i].id = i + 1;
		//sim->coders[i].t = something;
		sim->coders[i].last_compile_start = 0;
		sim->coders[i].compile_count = 0;
		sim->coders[i].burned_out = 0;
		sim->coders[i].sim = sim;
		i++;
	}
	connect_dongles(sim);
	return (0);
}

int	cleanup_dongles(t_simulation *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_cond_destroy(&sim->dongles[i].cond);
		pthread_mutex_destroy(&sim->dongles[i].lock);
		i++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
	return (1);
}
