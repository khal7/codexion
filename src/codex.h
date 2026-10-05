/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: khabouou <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 16:54:10 by khabouou          #+#    #+#             */
/*   Updated: 2026/08/10 16:54:12 by khabouou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

typedef struct s_coder		t_coder;
typedef struct s_dongle		t_dongle;
typedef struct s_simulation	t_simulation;

typedef struct s_heap
{
	t_coder	**arr;
	int		current_n;
	int		max_capacity;
}	t_heap;

typedef struct s_coder
{
	pthread_t		t;
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	t_simulation	*sim;
	long			last_compile_start;
	int				compile_count;
	int				id;
	int				burned_out;
	int				waiting_for_dongle;
	long			priority;
}	t_coder;

typedef struct s_dongle
{
	pthread_mutex_t	lock;
	int				id;
	int				is_available;
	long			waiting_count;
	long			last_released_time;
	// t_coder			*next_coder;
}	t_dongle;

typedef struct s_args
{
	long		number_of_coders;
	long		time_to_burnout;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	long		number_of_compiles_required;
	long		dongle_cooldown;
	char		*scheduler;
}	t_args;

typedef struct s_simulation
{
	t_args			args;
	t_coder			*coders;
	t_dongle		*dongles;
	t_heap			global_queue;
	pthread_mutex_t	queue_lock;
	pthread_mutex_t	p_lock;
	pthread_mutex_t	arrival_lock;
	pthread_mutex_t	state_lock;
	pthread_t		monitor;
	int				simulation_finished;
	long			start_time;
	int				arrival_counter;
}	t_simulation;

int		args_to_struct(int ac, char **av, t_args *ptr);
long	ft_atoi(char *str);
int		coder_init(t_args *arg, t_simulation *sim);
int		dongle_init(t_args *arg, t_simulation *sim);
int		cleanup_dongles(t_simulation *sim, int count);
int		thread_creation(t_simulation *sim);
int		request_dongle(t_coder *coder, t_dongle *first_dongle,
			t_dongle *second_dongle);
void	release_dongle(t_dongle *dongle, t_coder *coder);
void	connect_dongles(t_simulation *sim);
void	*monitor_thread(void *arg);
long	current_time(void);
void	printing(t_coder *coder, char *str);
int		compile_cycle(t_coder *coder, t_dongle *first_dongle,
			t_dongle *second_dongle);
void	*routine(void *arg);
int		sleep_control(t_simulation *sim, int sleep_time);
void	ft_swap(t_coder **a, t_coder **b);
void	heapify_up(t_heap *heap, int index);
void	heap_push(t_heap *heap, t_coder *coder);
void	heapify_down(t_heap *heap, int index);
t_coder	*heap_pop(t_heap *heap);

int		is_higher_priority(t_coder *a, t_coder *b);
int		shares_dongle(t_coder *a, t_dongle *low, t_dongle *high);
int		allowed_to_take(t_simulation *sim, t_coder *coder,
			t_dongle *low, t_dongle *high);
void	register_coder(t_coder *coder, t_simulation *sim);
int		try_take_dongles(t_coder *coder, t_dongle *low, t_dongle *high);
int		heap_find(t_heap *heap, t_coder *coder);
void	heap_remove_at(t_heap *heap, int index);
int		sim_is_finished(t_simulation *sim);
int		one_coder(t_simulation *sim, t_coder *coder, t_dongle *first_dongle);
void	sim_set_finished(t_simulation *sim, int value);

int		all_compile_done(t_simulation *sim);
int		burnout_check(t_simulation *sim, int i);
void	printing_three_lines(t_coder *coder);