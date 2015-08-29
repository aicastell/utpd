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

#ifndef _ERROR_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>

void
verror(const char *msg, va_list args);

void
error(const char *msg, ...);

void
error_exit(const char *msg, ...);

#endif

