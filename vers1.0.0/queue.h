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

#ifndef _C_QUEUE_H
#define _C_QUEUE_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "error.h"
#include "queue_item.h"

extern int g_verbose;

/**
* Circular queue.
* Produced items are added to the head of the queue.
* Consumed items are taken and removed from the tail */
struct queue {
	struct queue_item ** elems; /* pointer to a dynamic array of queue items */
    int max_size; /* maximum number of items on the queue */
	int head; /* index of the head element */
	int tail; /* index of the tail element */
    int nr_elems; /* current number of elements inserted into the queue */
    pthread_mutex_t mux; /* lock to protect internal syncronization */
    pthread_cond_t not_full; /* condition not full */
    pthread_cond_t not_empty; /* condition not empty */
};

/**
* Creates a new queue, initializing the fields */
void
queue_new(struct queue * q, int max_size);

/**
* Destroys a queue, deallocating resources previously allocated */
void
queue_del(struct queue * q);

/**
* Returns the number of elements in the queue */
int
queue_nr_elems(struct queue * q);

/**
* Returns true if queue is empty */
int
queue_is_empty(struct queue * q);

/**
* Returns true if queue is full */
int
queue_is_full(struct queue * q);

/**
* Inserts a new element on the head of the queue */
void
queue_head_add(struct queue * q, struct queue_item * item);

/**
* Deletes the oldest element on the queue, on the tail */
void
queue_tail_del(struct queue * q);

/**
* Returns the element on the tail */
struct queue_item *
queue_tail(struct queue * q);

/**
* Prints current status of queue on stdout for debugging purposes */
void
queue_debug(struct queue * q);

#endif

