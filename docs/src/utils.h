/*
 * utils.h
 *
 * This file is part of hl://Dig <https://github.com/andy5995/hldig>
 *
 *  Copyright (C) 2017-2018  Andy Alt (andy400-dev@yahoo.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include "simplecgen.h"

void
del_char (char **str, const char c);

void
parse_config (const char *cf, struct cfg *cfgopts);

short
bufchk (const char *str, ushort boundary);

int
trim (char *str);

void
add_newline (char *str);
