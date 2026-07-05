
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

