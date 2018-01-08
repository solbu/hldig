/*
 * utils.c
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

#include "utils.h"

/**
 * Erases characters from the beginning of a string
 * (i.e. shifts the remaining string to the left
 *
 * This function operates differently than del_char_shift_left
 *
 * example use shown in parse_config()
 */
void del_char (char **str, const char c)
{
  while (**str == c)
    ++(*str);

  return;
}

static void
parse_option (char *str, const char *l)
{
  char *value;
  if ((value = strchr (l, '=')) == NULL)
  {
    fprintf (stderr, "  %s\n", l);
    fprintf (stderr, "  :Error: in config file\n");
    exit (ERROR_CONFIG_LINE);
  }

  del_char (&value, '=');
  del_char (&value, ' ');

  trim (value);
  strcpy (str, value);

#ifdef DEBUG
PRINT_DEBUG ("value is '%s'\n", value);
PRINT_DEBUG ("str is '%s' at line %d\n", value, __LINE__);
#endif

  return;
}

void
parse_config (const char *cf, struct cfg *cfgopts)
{
  /* open the config file */
  FILE *cfg_p;
  if ((cfg_p = fopen (cf, "r")) == NULL)
  {
    fprintf (stderr, "  :Error: while opening %s\n", cf);
    exit (ERROR_CONFIG_OPEN);
  }

  char cfg_line[LINE_LEN_MAX];

  while (fgets (cfg_line, LINE_LEN_MAX, cfg_p) != NULL)
  {
    if (*cfg_line == '#')
      continue;

    if (strcmp (cfg_line, "site_title") > 0)
    {
      parse_option (cfgopts->site_title, cfg_line);
#ifdef DEBUG
PRINT_DEBUG ("cfg_line is %s at line %d\n", cfg_line, __LINE__);
PRINT_DEBUG ("cfgopts.site_title is '%s' at line %d\n", cfgopts->site_title, __LINE__);
#endif
      continue;
    }

    if (strcmp (cfg_line, "site_description") > 0)
    {
      parse_option (cfgopts->site_description, cfg_line);
#ifdef DEBUG
PRINT_DEBUG ("cfg_line is\n%s at line %d\n", cfg_line, __LINE__);
PRINT_DEBUG ("cfgopts.site_description is '%s' at line %d\n", cfgopts->site_description, __LINE__);
#endif
      continue;
    }
  }

  /* Close the config file */
  if (fclose (cfg_p) == EOF)
  {
    fprintf (stderr, "  :Error: while closing %s\n", cf);
    exit (ERROR_CONFIG_CLOSE);
  }

  return;
}

short
bufchk (const char *str, ushort boundary)
{
  /* str_part defines the first n characters of the string to display.
   * This assumes 10 will never exceed a buffer size. In this program,
   * there are no buffers that are <= 10 (that I can think of right now)
   */
  const ushort str_part = 10;
  static ushort len;
  len = strlen (str);

  if (len < boundary)
    return 0;

  /* TRANSLATORS:  "buffer" in the following instances refers to the amount
   * of memory allocated for a string  */
  printf ("  :Error: buffer overrun (segmentation fault) prevented.\n");
  printf ("If you think this may be a bug, please report it to %s\n", PACKAGE_BUGREPORT);

  /**
   * This will add a null terminator within the boundary specified by
   * display_len, making it possible to view part of the strings to help
   * with debugging or tracing the error.
   */
  static ushort display_len;
  display_len = 0;

  display_len = (boundary > str_part) ? str_part : boundary;

  char temp[display_len];
  strncpy (temp, str, display_len);
  temp[display_len] = '\0';


  fprintf (stderr, " <--> Displaying part of the string that caused the error <-->\n\n");
  fprintf (stderr, "%s\n\n", temp);

  return EXIT_BUF_ERR;
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

void
add_newline (char *str)
{
  static size_t len;
  len = strlen (str);
  str[len] = '\n';
  str[len + 1] = '\0';
}

