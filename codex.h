#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
typedef struct s_coder t_coder;
typedef struct s_dongle t_dongle;
typedef struct s_simulation t_simulation;

typedef struct s_heap
{
	t_coder **arr;
	int current_n;
	int	max_capacity;
	
} t_heap;

typedef struct s_coder
{
    int id;
    pthread_t t;
    t_dongle *left_dongle;
    t_dongle *right_dongle;
    long	last_compile_start;
    int	compile_count;
    int	burned_out;
	int	waiting_for_dongle;
	long	priority;
    t_simulation *sim;
} t_coder;

typedef struct s_dongle
{
	int	id;
	pthread_mutex_t lock;
	pthread_cond_t cond;
    int is_available;
    long waiting_count;
    long    last_released_time;
	t_heap	waiting_queue;
	t_coder *next_coder;
} t_dongle;

typedef struct s_args
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

typedef struct s_simulation
{
    t_args args;
    t_coder *coders;
    t_dongle *dongles;
	pthread_mutex_t p_lock;
	pthread_mutex_t arrival_lock;
	pthread_mutex_t state_lock;
	pthread_cond_t dongle_cond;
	pthread_t monitor;
    int simulation_finished;
	long	start_time;
	int	arrival_counter;
} t_simulation;

int	args_to_struct(int ac, char **av, t_args *ptr);
long	ft_atoi(char *str);
int coder_init(t_args *arg, t_simulation *sim);
int	dongle_init(t_args *arg, t_simulation *sim);
int	cleanup_dongles(t_simulation *sim, int count);
int	thread_creation(t_simulation *sim);
int	dongle_init(t_args *arg, t_simulation *sim);
int	request_dongle(t_coder *coder, t_dongle *first_dongle, t_dongle *second_dongle);
void	release_dongle(t_dongle *dongle, t_coder *coder);
void	connect_dongles(t_simulation *sim);
void	*monitor_thread(void *arg);
long	current_time(void);
void	printing(t_coder *coder, char *str);
int	compile_cycle(t_coder *coder, t_dongle *first_dongle, t_dongle *second_dongle);
void	*routine(void *arg);
int	sleep_control(t_simulation *sim, int sleep_time);
void	ft_swap(t_coder **a, t_coder **b);
void	heapify_up(t_heap *heap, int index);
void	heap_push(t_heap *heap, t_coder *coder);
void	heapify_down(t_heap *heap, int index);
t_coder	*heap_pop(t_heap *heap);
