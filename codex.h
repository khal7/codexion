
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

long	ft_atoi(char *str);

// typedef struct
// {
//     ...

// }s_coder;

typedef struct
{
    // s_coder *coder;
    long    number_of_coders;
    long    time_to_burnout;
    long    time_to_compile;
    long    time_to_debug;
    long    time_to_refactor;
    long    number_of_compiles_required;
    long    dongle_cooldown;
    char    *scheduler;
    

}t_args;
int  args_to_struct(int ac, char **av, t_args *ptr);