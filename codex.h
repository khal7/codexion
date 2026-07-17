#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

typedef struct s_coder t_coder;
typedef struct s_dongle t_dongle;
typedef struct s_simulation t_simulation;

typedef struct
{
    int id;
    pthread_t t;
    t_dongle *left_dongle;
    t_dongle *right_dongle;
    long    last_compile_start;
    int compile_count;
    int    burned_out;
    t_simulation *sim;
} t_coder;

typedef struct
{
	int	id;
	pthread_mutex_t lock;
	pthread_cond_t cond;
    int is_available;
    long waiting_count;
    long    last_released_time;
} t_dongle;

typedef struct
{
	long	number_of_coders;
	long    time_to_burnout;
	long    time_to_compile;
	long	time_to_debug;
	long	time_to_refactor;
	long	number_of_compiles_required;
	long   	dongle_cooldown;
	char     *scheduler;
} t_args;

typedef struct
{
    t_args args;
    t_coder *coders;
    t_dongle *dongles;
	pthread_mutex_t p_lock;
	pthread_t monitor;
    int simulation_finished;
	long	start_time;
} t_simulation;

int	args_to_struct(int ac, char **av, t_args *ptr);
long	ft_atoi(char *str);
int coder_initializtion(t_args *arg, t_simulation *sim);
int	dongle_initialization(t_args *arg, t_simulation *sim);
int	cleanup_dongles(t_simulation *sim, int count);