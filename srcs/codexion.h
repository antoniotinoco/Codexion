/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 00:22:14 by atinoco-          #+#    #+#             */
/*   Updated: 2026/08/04 01:12:54 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <limits.h>
# include <unistd.h>

typedef struct s_sim	t_sim;

/* ---- Scheduler policy ---- */
typedef enum e_scheduler
{
	SCHED_POLICY_FIFO,
	SCHED_POLICY_EDF
}	t_scheduler;

/* ---- Parsed command-line config ---- */
typedef struct s_config
{
	int			num_coders;
	long long	time_to_burnout;
	long long	time_to_compile;
	long long	time_to_debug;
	long long	time_to_refactor;
	int			compiles_required;
	long long	dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

/* ---- One coder's dongle-pair request ---- */
typedef struct s_request
{
	int			coder_id;
	long long	arrival_time;
	long long	deadline;
}	t_request;

/* ---- One dongle (shared resource) ---- */
typedef struct s_dongle
{
	int					id;
	int					available;
	long long			timestamp;
	pthread_mutex_t		lock;
	pthread_cond_t		cond;
	struct s_request	queue[2];
	int					queue_size;
}	t_dongle;

/* ---- One coder (thread) ---- */
typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	int				thread_created;
	int				compiles_done;
	long long		last_compile_start;
	long long		arrival_time;
	t_dongle		*left;
	t_dongle		*right;
	t_sim			*sim;
}	t_coder;

/* ---- Whole simulation state ---- */
typedef struct s_sim
{
	t_config		config;

	t_coder			*coders;
	t_dongle		*dongles;

	long long		start_time_ms;
	int				running;
	int				burned_out_id;
	int				threads_created;
	int				monitor_created;

	pthread_mutex_t	sim_lock;
	pthread_mutex_t	log_lock;
	pthread_mutex_t	start_lock;
	pthread_cond_t	start_cond;
	pthread_t		monitor;

	int				start_released;
	int				registered_coders;

	int				mutexes_ready;
	int				dongles_ready;
	int				start_aborted;
}	t_sim;

/* ---- args_parser.c ---- */
int			parse_args(t_sim *sim, int argc, char **argv);

/* ---- setup.c ---- */
int			setup_simulation(t_sim *sim, int argc, char **argv);

/* ---- setup_utils.c ---- */
void		free_sim_arrays(t_sim *sim);
void		cleanup_partial_setup(t_sim *sim);
void		destroy_sim_sync(t_sim *sim);
void		destroy_dongle_locks(t_sim *sim, int count);
void		thread_manager_cleanup(t_sim *sim);

/* ---- sim_state.c ---- */
long long	current_time_ms(void);
int			sim_is_running(t_sim *sim);
int			coder_should_continue(t_coder *coder);
void		update_compile_state(t_coder *coder, t_sim *sim);
void		sleep_with_stop(t_sim *sim, long long ms);

/* ---- thread_manager.c ---- */
int			thread_manager_start(t_sim *sim);

/* ---- thread_manager_join.c ---- */
int			thread_manager_join(t_sim *sim);

/* ---- scheduler_queue.c ---- */
void		enqueue(t_dongle *dongle, t_coder *coder);
void		dequeue(t_dongle *dongle);
void		wait_for_dongle(t_dongle *dongle, t_coder *coder);

/* ---- coder_loop.c ---- */
void		*coder_loop_run(t_coder *coder);

/* ---- dongle_grab.c ---- */
void		take_right_first(t_coder *coder);
void		take_left_first(t_coder *coder);
void		release_dongles(t_coder *coder);

/* ---- monitor.c ---- */
void		*watchdog_run(void *arg);

/* ---- clock_log.c ---- */
void		log_state(t_sim *sim, int coder_id, const char *state);
void		log_burnout_and_stop(t_sim *sim, int coder_id);

#endif
