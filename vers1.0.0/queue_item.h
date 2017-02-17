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

#ifndef _QUEUE_ITEM_H
#define _QUEUE_ITEM_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>

/**
* Item on the queue */
struct queue_item {
    char * frame; /* UDP frame sent by client */
    int frame_len; /* UDP frame length */
    struct sockaddr_in * from; /* client IP address */
    socklen_t * fromlen; /* from length */
};

void
queue_item_new(struct queue_item * item,
               char * frame, int frame_len,
               struct sockaddr_in * from, socklen_t * fromlen);

void
queue_item_del(struct queue_item * item);

#endif

