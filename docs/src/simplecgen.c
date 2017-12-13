/*
 * simplecgen.c: generates html files using the simplectemplate library
 *
 * Copyright 2017 Andy <andy400-004@yahoo.com>
 *
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <dirent.h>
#include "template_functions.h"

#ifndef VERSION
  #define VERSION "unversioned"
#endif

#define LINE_LEN_MAX 80
#define FILENAME_LEN_MAX 248

void
del_char_shift_left (char *str, char c);

int
trim (char *str);

int main(int argc, char **argv)
{
  if (argc > 1)
  {
    if (strcmp (argv[1], "--version") == 0)
    {
      printf ("%s\n", VERSION);
      return 0;
    }
    else
    {
      printf ("unsupported command line arguments given\n");
      return 1;
    }
  }

  struct dirent *dir_entry;
  DIR *infiles_dir;
  /* FIXME: need a check to make sure the directory and file exists
   * add more flexibility so the user can change this (hint: config file)
   */
  if ((infiles_dir = opendir ("infiles")) == NULL)
  {
    perror ("Error opening directory");
    return errno;
  }

  while ((dir_entry = readdir (infiles_dir)) != NULL)
  {
    /* Only read .sct files */
    if (strstr (dir_entry->d_name, ".sct") == NULL)
      continue;

    char input_file[FILENAME_LEN_MAX + 1];
    sprintf (input_file, "infiles/%s", dir_entry->d_name);
    printf ("processing %s\n", input_file);

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
    if (title == NULL)
    {
      printf ("%s has the wrong format\n", input_file);
      return 1;
    }

    del_char_shift_left (title, ':');

    trim (title);

    if (fseek (fp, 0, SEEK_END) != 0)
    {
      perror ("Error while seeking file");
      return errno;
    }

    size_t len = ftell (fp);

    char *contents;
    if ((contents = calloc (len, 1)) == NULL)
    {
      printf ("Unable to allocate memory\n");
      return 1;
    }

    rewind (fp);
    fread (contents, len, 1, fp);

    if (fclose (fp) == EOF)
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
      fclose (fp);
      free (contents);
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
    /* FIXME: need a check to make sure the directory and file exists
     * add more flexibility so the user can change this (hint: config file)
     */
    output = render_template_file ("templates/default.html", 2, data);

    len = strlen (output);
    output [len] = '\n';
    output [len + 1] = '\0';

    /* truncate the .sct extension */
    input_file[strlen (input_file) - 4] = '\0';

    char dest_file[FILENAME_LEN_MAX + 1];
    sprintf (dest_file, "%s.html", basename (input_file));

    if ((fp = fopen (dest_file, "w")) == NULL)
    {
      perror ("Error opening");
      free (contents);
      return errno;
    }

    // printf ("%s\n", output);

    fwrite (output, strlen (output), 1, fp);

    if (fclose (fp) == EOF)
    {
      perror ("Error while closing file\n");
    }

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

  while (c == ' ' || c == '\t' || c == '\n' || c == EOF)
  {
    str[n] = '\0';
    n--;
    c = str[n];
  }

  return n;
}
