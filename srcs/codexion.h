/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/21 00:22:14 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/21 00:22:14 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>

/* ---- Scheduler policy ---- */
typedef enum e_scheduler
{
	SCHED_FIFO,
	SCHED_EDF
}	t_scheduler;

/* ---- Parsed command-line config (never changes after setup) ---- */
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

/* ---- One dongle (shared resource) ---- */
typedef struct s_dongle
{
	int				id;
	int				is_taken;
	long long		available_at_ms;	/* cooldown expiry timestamp */
	pthread_mutex_t	lock;
}	t_dongle;

/* ---- One coder (thread) ---- */
typedef struct s_coder
{
	int				id;				/* 1..num_coders */
	pthread_t		thread;
	int				thread_created;	/* for partial-cleanup safety */
	int				compiles_done;
	long long		last_compile_start;
	long long		ticket;			/* arrival order, for FIFO */
	t_dongle		*left;
	t_dongle		*right;
	t_sim			*sim;
}	t_coder;

typedef struct s_waiter
{
	int			coder_idx;
	long long	ticket;
	long long	priority;
	long long	tie;
}	t_waiter;

typedef struct s_request
{
	t_dongle	*first;
	t_dongle	*second;
	long long	ticket;
}	t_request;

/* ---- Whole simulation state (no globals; passed everywhere) ---- */
typedef struct s_sim
{
	t_config		config;

	t_coder			*coders;
	t_dongle		*dongles;

	long long		start_time_ms;
	int				running;		/* 0 once simulation should stop */
	int				burned_out_id;	/* 0 if none, else coder id */
	int				threads_created;	/* how many coder threads succeeded */
	int				monitor_created;

	pthread_mutex_t	sim_lock;		/* protects running/burned_out_id */
	pthread_mutex_t	log_lock;		/* protects printf/write */
	pthread_mutex_t	start_lock;		/* protects start barrier */
	pthread_cond_t	start_cond;		/* start barrier */
	pthread_cond_t	dongle_cond;
	pthread_t		monitor;
	
	int				start_released;

	int				mutexes_ready;	/* were the 3 mutexes above init'd? */
	int				dongles_ready;	/* how many dongle mutexes init'd */
	int				start_aborted;
	
	t_request		*requests;		/* one per coder */
	t_waiter		**dongle_heaps;	/* one heap array per dongle */
	int				*heap_sizes;	/* one size counter per dongle */
	long long		next_ticket;
}	t_sim;

/* ---- main.c ---- */
int		setup_simulation(t_sim *sim, int argc, char **argv);
void	free_sim_arrays(t_sim *sim);
int		thread_manager_start(t_sim *sim);
int		thread_manager_join(t_sim *sim);
void	thread_manager_cleanup(t_sim *sim);

/* ---- args_parser.c ---- */
int		parse_args(t_sim *sim, int argc, char **argv);

/* ---- sim_sync.c ---- */
int		init_sim_sync(t_sim *sim);
void	destroy_sim_sync(t_sim *sim);
int		init_coders_and_dongles(t_sim *sim);
void	destroy_dongle_locks(t_sim *sim, int count);

#endif
