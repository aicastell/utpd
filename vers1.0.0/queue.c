/**
* utpd: UDP to TCP proxy daemon
* Copyright (C) 2011
* Angel Ivan Castell Rovira <al004140 at gmail dot com>
*
* This file is part of utpd.
*
* utpd is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* utpd is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with utpd.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "queue.h"

void
queue_new(struct queue * q, int max_size)
{
    q->head = 0;
    q->tail = 0;
    q->nr_elems = 0;
    q->max_size = max_size;
    q->elems = (struct queue_item **) malloc(q->max_size * sizeof(struct queue_item *));
    if (q->elems == NULL)
        error_exit("Memory allocation failed!\n");

    int i;
    for (i = 0; i < q->max_size; i++)
        q->elems[i] = NULL;

    pthread_mutex_init(&q->mux, NULL);
    pthread_cond_init(&q->not_full, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

void
queue_del(struct queue * q)
{
    free(q->elems);

    pthread_mutex_destroy(&q->mux);
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
}

int
queue_nr_elems(struct queue * q)
{
    return q->nr_elems;
}

int
queue_is_empty(struct queue * q)
{
    return (q->nr_elems == 0);
}

int
queue_is_full(struct queue * q)
{
    return (q->nr_elems == q->max_size);
}

void
queue_head_add(struct queue * q, struct queue_item * item)
{
    if (!queue_is_full(q)) {
        if (g_verbose > 0)
            fprintf(stdout, "queuing \"%s\" in position %d\n", item->frame, q->head);
        q->elems[q->head] = item;
        q->head = (q->head + 1) % q->max_size;
        pthread_mutex_lock(&q->mux);
        {
            q->nr_elems++;
            if (queue_is_full(q))
                pthread_cond_wait(&q->not_full, &q->mux);
            pthread_cond_signal(&q->not_empty);
        }
        pthread_mutex_unlock(&q->mux);
    } else {
		fprintf(stdout, "[warning]: element inserted discarted, queue is full\n");
	}
}

void
queue_tail_del(struct queue * q)
{
	if (!queue_is_empty(q)) {
        queue_item_del(q->elems[q->tail]);
	    free(q->elems[q->tail]);
	    q->elems[q->tail] = NULL;
	    q->tail = (q->tail + 1) % q->max_size;
        pthread_mutex_lock(&q->mux);
        {
            q->nr_elems--;
            if (queue_is_empty(q))
                pthread_cond_wait(&q->not_empty, &q->mux);
            pthread_cond_signal(&q->not_full);
        }
        pthread_mutex_unlock(&q->mux);
	}
}


struct queue_item *
queue_tail(struct queue * q)
{
    return q->elems[q->tail];
}

void
queue_debug(struct queue * q)
{
	int i;

	if (!queue_is_full(q)) {
		for (i = q->tail; i != q->head; i = (i + 1) % q->max_size)
			printf("==> %s\n", q->elems[i]->frame);
	} else {
		printf("%s\n", q->elems[q->tail]->frame);
		for (i = (q->tail + 1) % q->max_size; i != q->head; i = (i + 1) % q->max_size)
			printf("==> %s\n", q->elems[i]->frame);
	}
}

