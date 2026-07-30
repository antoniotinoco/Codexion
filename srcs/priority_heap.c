/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   priority_heap.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: atinoco- <atinoco-@student.42lisboa.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 10:52:15 by atinoco-          #+#    #+#             */
/*   Updated: 2026/07/27 10:52:15 by atinoco-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/* Swap two heap elements. */
static void	heap_swap(t_waiter *a, t_waiter *b)
{
	t_waiter	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

/* Compare two waiters: lower priority first, then tie, then ticket. */
static int	heap_is_less(t_waiter c, t_waiter p)
{
	if (c.priority < p.priority)
		return (1);
	if (c.priority > p.priority)
		return (0);
	if (c.tie < p.tie)
		return (1);
	if (c.tie > p.tie)
		return (0);
}

/* Insert one waiter into the heap and restore heap order (sift-up). */
void	heap_push(t_waiter *heap, int *size, t_waiter elem)
{
	int	i;
	int	parent;

	i = *size;
	(*size)++;
	heap[i] = elem;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!heap_is_less(heap[i], heap[parent]))
			break ;
		heap_swap(&heap[i], &heap[parent]);
		i = parent;
	}
}

/* Return the best (lowest-priority) waiter currently in the heap. */
t_waiter	heap_peek(t_waiter *heap, int size)
{
	t_waiter	empty;

	if (size == 0)
	{
		empty.coder_idx = -1;
		empty.priority = 0;
		empty.ticket = 0;
		empty.tie = 0;
		return (empty);
	}
	return (heap[0]);
}
