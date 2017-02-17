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

#include <string.h>
#include "queue.h"
#include "queue_item.h"

int g_verbose = 1;
const char *g_program_name = 0;

int main(void)
{
    /* Create frames for the test */
    char * s0 = (char *)malloc(128);
    char * s1 = (char *)malloc(128);
    char * s2 = (char *)malloc(128);
    char * s3 = (char *)malloc(128);

    strncpy(s0, "Hello world 0", 128);
    strncpy(s1, "Hello world 1", 128);
    strncpy(s2, "Hello world 2", 128);
    strncpy(s3, "Hello world 3", 128);

    /* Create queue items */
    struct queue_item * item0 = (struct queue_item *) malloc(sizeof(struct queue_item));
    struct queue_item * item1 = (struct queue_item *) malloc(sizeof(struct queue_item));
    struct queue_item * item2 = (struct queue_item *) malloc(sizeof(struct queue_item));
    struct queue_item * item3 = (struct queue_item *) malloc(sizeof(struct queue_item));

    item0->frame = s0;
    item0->from = NULL;
    item0->fromlen = NULL;
    item1->frame = s1;
    item1->from = NULL;
    item1->fromlen = NULL;
    item2->frame = s2;
    item2->from = NULL;
    item2->fromlen = NULL;
    item3->frame = s3;
    item3->from = NULL;
    item3->fromlen = NULL;

    /* start the test */
    struct queue q;

    queue_new(&q, 10);

    queue_debug(&q);
    printf("queue is empty = %d\n", queue_is_empty(&q));
    printf("queue is full = %d\n", queue_is_full(&q));

    queue_head_add(&q, item0);
    queue_head_add(&q, item1);
    queue_head_add(&q, item2);
    queue_head_add(&q, item3);

    queue_debug(&q);
    printf("queue is empty = %d\n", queue_is_empty(&q));
    printf("queue is full = %d\n", queue_is_full(&q));

    queue_tail_del(&q);
    queue_tail_del(&q);

    queue_debug(&q);
    printf("queue is empty = %d\n", queue_is_empty(&q));
    printf("queue is full = %d\n", queue_is_full(&q));

    queue_tail_del(&q);
    queue_tail_del(&q);

    queue_debug(&q);
    printf("queue is empty = %d\n", queue_is_empty(&q));
    printf("queue is full = %d\n", queue_is_full(&q));

    queue_del(&q);

    return (0);
}

