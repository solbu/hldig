/*

    Part of the ht://Dig package   <http://www.htdig.org/>
    Copyright (c) 1999, 2000 The ht://Dig Group
    For copyright details, see the file COPYING in your distribution
    or the GNU General Public License version 2 or later
    <http://www.gnu.org/copyleft/gpl.html>
    
*/

#include <config.h>

#if HAVE_STDBOOL_H
# include <stdbool.h>
#else
# if ! HAVE__BOOL
#  ifdef __cplusplus
  typedef bool _Bool;
#  else
  typedef unsigned char _Bool;
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif

