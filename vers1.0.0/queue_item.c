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

#include "queue_item.h"

void
queue_item_new(struct queue_item * item,
               char * frame, int frame_len,
               struct sockaddr_in * from, socklen_t * fromlen)
{
    item->frame = frame;
    item->frame_len = frame_len;
    item->from = from;
    item->fromlen = fromlen;
}

void
queue_item_del(struct queue_item * item)
{
    free(item->frame);
    free(item->from);
    free(item->fromlen);
}

