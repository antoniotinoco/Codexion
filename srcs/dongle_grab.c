/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle_grab.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 10:48:26 by atinoco-          #+#    #+#             */
/*   Updated: 2026/08/09 22:17:04 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Take left then right dongle. */
void	take_left_first(t_coder *coder)
{
	coder->arrival_time = current_time_ms();
	wait_for_dongle(coder->left, coder);
	if (!coder_should_continue(coder))
		return ;
	log_state(coder->sim, coder->id, "has taken a dongle");
	wait_for_dongle(coder->right, coder);
	if (!coder_should_continue(coder))
		return ;
	log_state(coder->sim, coder->id, "has taken a dongle");
}

/* Take right then left dongle. */
void	take_right_first(t_coder *coder)
{
	coder->arrival_time = current_time_ms();
	wait_for_dongle(coder->right, coder);
	if (!coder_should_continue(coder))
		return ;
	log_state(coder->sim, coder->id, "has taken a dongle");
	wait_for_dongle(coder->left, coder);
	if (!coder_should_continue(coder))
		return ;
	log_state(coder->sim, coder->id, "has taken a dongle");
}

/* Release both dongles and start their cooldowns. */
void	release_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->left->lock);
	coder->left->timestamp = current_time_ms();
	coder->left->available = 1;
	pthread_cond_broadcast(&coder->left->cond);
	pthread_mutex_unlock(&coder->left->lock);
	pthread_mutex_lock(&coder->right->lock);
	coder->right->timestamp = current_time_ms();
	coder->right->available = 1;
	pthread_cond_broadcast(&coder->right->cond);
	pthread_mutex_unlock(&coder->right->lock);
}
