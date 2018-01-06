/*
 * simplecgen.c: generates html files using the simplectemplate library
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

#include <libgen.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>

#include "simplecgen.h"
#include "utils.h"

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

  char *bin_dir = dirname (argv[0]);

  char cfg_file[PATH_MAX + 1];
  sprintf (cfg_file, "%s/%s", bin_dir, CONFIG_FILE_BASE);

  char site_title[120];

#ifdef DEBUG
  PRINT_DEBUG ("config file = %s\n", cfg_file);
#endif

  /* As more config options are added, it will be easier to pass
   * a single structure, as opposed to all the different config
   * variables
   */
  parse_config  (cfg_file, site_title);

#ifdef DEBUG
  PRINT_DEBUG ("site_title = %s\n", site_title);
#endif

  FILE *tail_fp;
  if ((tail_fp = fopen ("templates/tail.html", "r")) == NULL)
  {
    perror ("Error opening file");
    return errno;
  }

  fseek (tail_fp, 0, SEEK_END);
  size_t tail_size = ftell (tail_fp);
  rewind (tail_fp);

  char *output_tail;
  if ((output_tail = calloc (tail_size + 1, sizeof (char))) == NULL)
  {
    printf ("Unable to allocate memory\n");
    return 1;
  }

  fread (output_tail, tail_size, 1, tail_fp);

  if (fclose (tail_fp) == EOF)
  {
    perror ("Error closing file");
    return errno;
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

    /* get the title line */
    char title_line[LINE_LEN_MAX + 1];
    if (fgets (title_line, LINE_LEN_MAX, fp) == NULL)
    {
      perror ("Error getting line");
      fclose (fp);
      return 1;
    }

    char *title = strchr (title_line, ':');

    if (title == NULL)
    {
      printf ("%s has the wrong format\n", input_file);
      return 1;
    }

    del_char (&title, ':');
    trim (title);

    /* get the layout line */
    char layout_line[LINE_LEN_MAX + 1];

    if (fgets (layout_line, LINE_LEN_MAX, fp) == NULL)
    {
      perror ("Error getting line");
      fclose (fp);
      return 1;
    }

    char *layout = strchr (layout_line, ':');

    if (layout == NULL)
    {
      printf ("%s has the wrong format\n", input_file);
      return 1;
    }

    del_char (&layout, ':');
    trim (layout);

    /* Go back to the beginning of the file */
    if (fseek (fp, 0, SEEK_END) != 0)
    {
      perror ("Error while seeking file");
      return errno;
    }

    size_t len = ftell (fp);

    char *contents;
    if ((contents = calloc (len + 1, 1)) == NULL)
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

    /* find the first ocurrence of "-" */
    // char *body;
    char *body = strchr (contents, '-');

    if (body == NULL)
    {
      printf ("%s has the wrong format\n", input_file);
      fclose (fp);
      free (contents);
      return 1;
    }

    while (body[0] == '-' || body[0] == '\n')
      del_char (&body, body[0]);

    len = strlen (body) - 1;

    /* truncate anything, especially newlines */
    while (body[len] != '>')
    {
      body[len] = '\0';
      len--;
    }

    /* Because the head and layout templates are split now, this is not used
     *
    const char *data[] = {
      "title", title,
      "body", body
    };
    *
    */

    const char *title_data[] = {
      "title", title
    };

    const char *body_data[] = {
      "body", body
    };

    char layout_template[FILENAME_LEN_MAX];
    sprintf (layout_template, "templates/%s.html", layout);

    FILE *fp_layout;
    if ((fp_layout = fopen (layout_template, "r")) == NULL)
    {
      printf ("  :Error: layout: %s not found\n", layout_template);
      return 1;
    }

    /* FIXME: This is a temporary fix to try to solve a malloc error
     * @escottalexander is having on his os x system.
     */
    char output_head[2000000];
    char output_layout[2000000];

    /* FIXME: need a check to make sure the directory and file exists
     * add more flexibility so the user can change this (hint: config file)
     */
    char *output_head_tmp = render_template_file ("templates/head.html", 1, title_data);
    strcpy (output_head, output_head_tmp);
    add_newline (output_head);

    char *output_layout_tmp = render_template_file (layout_template, 1, body_data);
    strcpy (output_layout, output_layout_tmp);
    add_newline (output_layout);

    char output[strlen (output_head) + strlen (output_layout) +
        tail_size + 1];
    sprintf (output, "%s%s%s", output_head, output_layout, output_tail);

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

    fwrite (output, strlen (output), 1, fp);

    if (fclose (fp) == EOF)
    {
      perror ("Error while closing file\n");
    }

    free (title);
    free (body);
    free (layout);
    free (contents);
    free (output_head_tmp);
    free (output_layout_tmp);
  }

  free (output_tail);

  if (closedir (infiles_dir) != 0)
  {
    perror ("Error closing directory:");
    return errno;
  }

  return 0;
}
