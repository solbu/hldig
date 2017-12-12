/*
 * generate_site.c
 *
 * Copyright 2017 Andy <andy@oceanus>
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <dirent.h>
#include "template_functions.h"

#define LINE_LEN_MAX 80
#define FILENAME_LEN_MAX 248

void
del_char_shift_left (char *str, char c);

int
trim (char *str);

int main(int argc, char **argv)
{
  struct dirent *dir_entry;
  DIR *infiles_dir;
  if ((infiles_dir = opendir ("../infiles")) == NULL)
  {
    perror ("Error opening directory");
    return errno;
  }

  while ((dir_entry = readdir (infiles_dir)) != NULL)
  {
    /* Only read .md files */
    if (strstr (dir_entry->d_name, ".sct") == NULL)
      continue;

    char input_file[FILENAME_LEN_MAX + 1];
    sprintf (input_file, "../infiles/%s", dir_entry->d_name);
    printf ("%s\n", input_file);
    // continue;

    FILE *fp = fopen (input_file, "r");
    if (fp == NULL)
    {
      perror ("Error opening");
      return errno;
    }

    char line[LINE_LEN_MAX + 1];
    if (fgets (line, LINE_LEN_MAX, fp) == NULL)
    {
      perror ("Error getting line");
      fclose (fp);
      return 1;
    }

    char *title;

    title = strchr (line, ':');

    del_char_shift_left (title, ':');

    trim (title);

    fseek (fp, 0, SEEK_END);
    int len = ftell (fp);
    char *contents = (char *)malloc (len + 1);
    rewind (fp);

    fread (contents, len, 1, fp);

    if (fclose (fp) != 0)
    {
      perror ("Error  closing");
      free (contents);
      return errno;
    }

    char *body;

    body = strchr (contents, '-');

    if (body == NULL)
    {
      printf ("%s has the wrong format\n", input_file);
      return 1;
    }

    while (body[0] == '-' || body[0] == '\n')
      del_char_shift_left (body, body[0]);

    len = strlen (body) - 1;

    /* truncate anything, especially newlines */
    while (body[len] != '>')
    {
      body[len] = '\0';
      len--;
    }

    const char *data[] = {
      "title", title,
      "body", body
    };

    static char *output;
    output = render_template_file ("../templates/default.html", 2, data);

    len = strlen (output);
    output [len + 1] = '\0';
    output [len] = '\n';
    // output [len] = EOF;

    /* truncate the .sct extension */
    input_file[strlen (input_file) - 4] = '\0';

    char dest_file[FILENAME_LEN_MAX + 1];
    sprintf (dest_file, "%s.html", basename (input_file));

    fopen (dest_file, "w");

    fwrite (output, strlen (output), sizeof (char), fp);

    fclose (fp);

    free (contents);

  }

  return 0;
}

/**
 * Erases characters from the beginning of a string
 * (i.e. shifts the remaining string to the left
 */
void del_char_shift_left (char *str, char c)
{
  static int c_count;
  c_count = 0;

  /* count how many instances of 'c' */
  while (str[c_count] == c)
    c_count++;

  /* if no instances of 'c' were found... */
  if (!c_count)
    return;

  static int len;
  len = strlen (str);
  static int pos;

  for (pos = 0; pos < len - c_count; pos++)
    str[pos] = str[pos + c_count];

  str[len - c_count] = '\0';

  return;
}

/**
 * trim: remove trailing blanks, tabs, newlines
 * Adapted from The ANSI C Programming Language, 2nd Edition (p. 65)
 * Brian W. Kernighan & Dennis M. Ritchie
 */
int
trim (char *str)
{
  static int n;

  n = strlen (str);

  char c;
  c = str[n];

  if (c != '\0')
  {
    printf ("null terminator not found\n");
  }

  n--;
  c = str[n];
  // n = 0;
  /* while (str[n] != '\0')
  {
    printf ("%d\n", c);
    n++;
  }
  return 0; */


  while (c == ' ' || c == '\t' || c == '\n' || c == EOF)
  {
    str[n] = '\0';
    n--;
    c = str[n];
  }

  return n;
}
