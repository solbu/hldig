/*
 * simplecgen.h: generates html files using the simplectemplate library
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

#ifndef _INC_SIMPLECGEN_H
#define _INC_SIMPLECGEN_H

#define _XOPEN_SOURCE 600
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "template_functions.h"

#ifndef PATH_MAX
#   define PATH_MAX 1024
#endif

#ifndef VERSION
  #define VERSION "unversioned"
#endif

#ifdef DEBUG
#   define PRINT_DEBUG printf ("[DEBUG]:"); printf
#endif

#define LINE_LEN_MAX 161
#define FILENAME_LEN_MAX 248

#define CONFIG_FILE_BASE "simplecgen.conf"

typedef unsigned short int ushort;

/* This enum list will get used more later
 */
enum {
  EXIT_BUF_ERR = 100,
  ERROR_CONFIG_OPEN,
  ERROR_CONFIG_CLOSE,
  ERROR_CONFIG_LINE
};

typedef struct cfg {
  char site_title[LINE_LEN_MAX];
  char site_description[LINE_LEN_MAX];
} struct_cfg;

#endif
