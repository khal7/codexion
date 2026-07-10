
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

int coder_initializtion(t_args *arg, t_simulation *sim)
{
	sim->args = *arg;
	int	i;
	sim->coders = NULL;
	sim->coders = malloc(sizeof(t_coder) * arg->number_of_coders);
	if (!sim->coders)
	{
		free(sim->coders);
		return (1);
	}
	i = 0;
	while (i < arg->number_of_coders)
	{
		sim->coders[i].id = i + 1;
		//sim->coders[i].t = something;
		//sim->coders[i].left_dongle = something;
		//sim->coders[i].right_dongle = something;
		sim->coders[i].last_compile_start = 0;
		sim->coders[i].compile_count = 0;
		sim->coders[i].burned_out = 0;
		sim->coders[i].sim = sim;
		i++;
	}
	return (0);
}

int	dongle_initialization(t_args *arg, t_simulation *sim)
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
		{
			free(sim->dongles);
			return (1);
		}
		if (pthread_cond_init(&sim->dongles[i].cond, NULL))
		{
			free(sim->dongles);
			return (1);
		}
		sim->dongles[i].is_available = 1;
		sim->dongles[i].waiting_count = 0;
		sim->dongles[i].last_released_time = 0;
		i++;
	}
	return (0);
}








// int if_digit(int ac, char **av)
// {
//     int i;
//     int j;

//     i = 1;

//     while (i < (ac - 1))
//     {
//         j = 0;
//         if (!av[i][j])
//             return (1);
//         while (av[i][j])
//         {
//             if (!(av[i][j] >= '0' && av[i][j] <= '9'))
//                 return (1);
//             j++;
//         }
//         i++;
//     }
//     return (0);
// }

