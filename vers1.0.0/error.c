/**
* wheebop: UDP to TCP proxy daemon
* Copyright (C) 2011
* Angel Ivan Castell Rovira <al004140 at gmail dot com>
*
* This file is part of wheebop.
*
* wheebop is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* wheebop is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with wheebop.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "error.h"

extern const char *g_program_name;

void
verror(const char *msg, va_list args)
{
    char buffer[1024];

    vsprintf(buffer, msg, args);
    fflush(stdout);
    fprintf(stderr, "%s: %s\n", g_program_name, buffer);
    fflush(NULL);
}

void
error(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    verror(msg, args);
    va_end(args);
}

void
error_exit(const char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    verror(msg, args);
    va_end(args);

    exit (1);
}

