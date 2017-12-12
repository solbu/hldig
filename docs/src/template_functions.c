/*
 * template_functions.c
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

#ifndef UNITTESTS
	#define STATIC static
#else
	#define STATIC
#endif

#ifndef TEMPLATE_FUNCTIONS_H
#include "template_functions.h"
#endif

/**
 * Read the contents of a file
 * filename: Relative path of the file to read
 * returns: null-terminated string of the file contents
 */
STATIC char* read_file_contents(const char* filename) {
	// Open ze file
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) return NULL;

	// Get the content length
	fseek(fp, 0, SEEK_END);
	long length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Get the contents
	char* contents = malloc(length+1);
	memset(contents, 0, length+1);

	fread(contents, 1, length, fp);

	// No need to check if we fclose properly?
	// Even if the call fails, the stream passed as parameter will no longer be associated with the file nor its buffers.
	fclose(fp);

	if(!contents) return contents;

	// Remove the newline from the end but preserve a valid pointer so we can tell the difference between an "error" (when NULL) and simply an empty file.
	if(length) {
		char* removednl = malloc(length);
		memset(removednl, 0, length);
		memcpy(removednl, contents, length-1);
		free(contents);
		return removednl;
	}

	return contents;
}

// "Stolen" from https://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
/**
 * Replace all occurances of rep
 * orig: text to perfom search and replace on
 * rep: text to search for
 * with: text to replace with
 * returns: replaced text
 */
// You must free the result if result is non-NULL.
STATIC char* str_replace(char* orig, const char* rep, const char* with) {
	char* result;  // the return string
	char* ins;     // the next insert point
	char* tmp;     // varies
	int len_rep;   // length of rep (the string to remove)
	int len_with;  // length of with (the string to replace rep with)
	int count;     // number of replacements

	// sanity checks and initialization
	if(!orig || !rep) return NULL;

	len_rep = strlen(rep);
	if(len_rep == 0) return NULL; // empty rep causes infinite loop during count

	if(!with) with = "";
	len_with = strlen(with);

	// count the number of replacements needed
	ins = orig;     // the next insert point
	for(count = 0; (tmp = strstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if(!result) return NULL;

	// first time through the loop, all the variable are set correctly from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of rep in orig
	//    orig points to the remainder of orig after "end of rep"
	while(count--) {
			ins = strstr(orig, rep);
			int len_front = ins - orig;
			tmp = strncpy(tmp, orig, len_front) + len_front;
			tmp = strcpy(tmp, with) + len_with;
			orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy(tmp, orig);

	return result;
}

STATIC const char* get_last_token(const char* current, unsigned int token_count, const char* tokens[]) {
	unsigned int remaining_len = strlen(current);
	const char* next_token = &current[remaining_len];
	const char* last_delimiter = NULL;

	unsigned int i = 0;
	for(; i < token_count; ++i) {
		if(remaining_len >= strlen(tokens[i])) {
			char* subject_token = strstr(current, tokens[i]);
			if(subject_token != NULL && subject_token < next_token) {
				next_token = subject_token;
				last_delimiter = tokens[i];
			}
		}
	}

	return last_delimiter;
}

STATIC const char* my_strtok(const char* current, unsigned int token_count, const char* tokens[]) {
	unsigned int remaining_len = strlen(current);
	const char* next_token = &current[remaining_len];

	unsigned int i = 0;
	for(; i < token_count; ++i) {
		if(remaining_len >= strlen(tokens[i])) {
			char* subject_token = strstr(current, tokens[i]);

			if(subject_token != NULL && subject_token < next_token) {
				next_token = subject_token;
			}
		}
	}

	if(next_token < &current[remaining_len]) return next_token;

	return NULL;
}

STATIC int get_surrounded_with(const char* template, const char* open, const char* close, int* start, int* length) {
	*start = -1;
	*length = -1;

	const char* tokens[] = { open, close };

	char template_copy[strlen(template) + 1];
	strcpy(template_copy, template);

	const char* current = my_strtok(template_copy, 2, tokens);
	while(current != NULL) {
		const char* last_token = get_last_token(current, 2, tokens);

		if(last_token == open) *start = current - template_copy + strlen(open);
		else if(*start > -1 && last_token == close) *length = (current - template_copy) - *start;

		if(*start > -1 && *length > -1) {
			return 1;
		}

		current = my_strtok(&current[1], 2, tokens);
	}

	return 0;
}

char* my_render_template(const char* template_data, int len, const char* data[], struct RenderOptions options) {
	const char* keys[len];
	const char* values[len];
	unsigned int i;
	for(i = 0; i < len; i++) {
		keys[i] = (char *)data[i*2];
		values[i] = (char *)data[i*2+1];
	}

	// Get the options and set defaults if they aren't set.
	const char* open = options.placeholder_open;
	const char* close = options.placeholder_close;
	const char* data_open = options.data_open;

	if(open == NULL) open = "{{";
	if(close == NULL) close = "}}";
	if(data_open == NULL) data_open = "";

	int template_length = strlen(template_data) + 1;
	char* output = malloc(template_length);
	strcpy(output, template_data);

	int start = -1, length = -1;
	while(get_surrounded_with(output, open, close, &start, &length)) {
		char matched[length+1];
		memset(matched, 0, length + 1);
		strncpy(matched, output + start, length);

		char toreplace[strlen(open) + strlen(matched) + strlen(close) + 1];
		strcpy(toreplace, open);
		strcat(toreplace, matched);
		strcat(toreplace, close);

		unsigned int i = 0;
		for(; i < len; ++i) {
			unsigned int keylen = strlen(data_open) + strlen(keys[i]);
			char key[keylen + 1];
			strcpy(key, data_open);
			strcat(key, keys[i]);

			if(strncmp(matched, key, keylen) == 0) {
				char* replaced = str_replace(output, toreplace, values[i]);
				free(output);
				output = replaced;
				break;
			}
		}

		if(i >= len) {
			char* replaced = str_replace(output, toreplace, "");
			free(output);
			output = replaced;
		}
	}

	return output;
}

char* render_template(const char* template_data, int len, const char* data[]) {
	return my_render_template(template_data, len, data, (struct RenderOptions){});
}

char* my_render_template_file(const char* filename, int len, const char* data[], struct RenderOptions options) {
	char* contents = read_file_contents(filename);
	if(!contents) return NULL;

	char* rendered = my_render_template(contents, len, data, options);
	free(contents);

	return rendered;
}

char* render_template_file(const char* filename, int len, const char* data[]) {
	return my_render_template_file(filename, len, data, (struct RenderOptions){});
}
