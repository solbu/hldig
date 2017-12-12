/*
 * template_functions.h
 * https://github.com/dafky2000/simplectemplate
 * Provides HTML template functionality for https://github.com/andy5995/mhwkb
 *
 * Copyright 2017 Daniel Kelly <myself@danielkelly.me>
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
 */

#ifndef TEMPLATE_FUNCTIONS_H
#define TEMPLATE_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct RenderOptions {
	const char* placeholder_open;    // == "{{"
	const char* placeholder_close;   // == "}}"
	const char* data_open;

	/* const char* condition_delimeter; // == " " */
	/* const char* section_open;        // == "#" */
	/* const char* section_end;         // == "/" */
} options;

/**
 * Render a template with arrays of key/value pairs
 * template_data: template string
 * len: number of key/value pairs
 * keys: array of const char* keys to replace
 * values: array of const char* values to replace into template
 * options: RenderOptions structure with values for rendering templates
 * returns: rendered template, null terminated
 */
char* my_render_template (const char* template_data, int len, const char* data[], struct RenderOptions options);
char* render_template (const char* template_data, int len, const char* data[]);

/**
 * Render a template with arrays of key/value pairs from a file
 * filename: name of file to open as the template
 * len: number of key/value pairs
 * keys: array of const char* keys to replace
 * values: array of const char* values to replace into template
 * options: RenderOptions structure with values for rendering templates
 * returns: rendered template, null terminated
 */
char* my_render_template_file (const char* filename, int len, const char* data[], struct RenderOptions options);
char* render_template_file (const char* filename, int len, const char* data[]);

#endif
