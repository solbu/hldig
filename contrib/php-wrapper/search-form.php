<?
   # This is a php wrapper for use with htdig (http://www.htdig.org/)
   #
   # Copyright (c) 2001 DVL Software Limited
   # http://www.dvl-software.com/
   #
   # this file contains the form used within search.php
   #

function DisplayOption($value, $tag, $default) {
#
# This function creates a option and marks that option as selected
# if the tag value matches the default value provided.
# If the time is:
#
#   "and",    "All",     "and"
#
# the output would be:
#
#   <option value="and" selected>All</option>
#
# for "and",    "All",     "boolean"
#
# the output would be:
#
#   <option value="and">All</option>
#

   $result = '<option value="' . $value . '"';
   if ($value == $default) {
      $result .= ' selected';
   }
   $result .= '>' . $tag . '</option>';

   return $result;
}

#echo '<br>method = ' . $method . "<br>\n";
#echo 'format = ' . $format . "<br>\n";
#echo 'sort   = ' . $sort   . "<br>\n";

#$method='boolean';

if ($Debug) {
   echo '<BR>in search-form.php<BR>';
   echo 'method = ' . $method . "<br>\n";
   echo 'format = ' . $format . "<br>\n";
   echo 'sort   = ' . $sort   . "<br>\n";
}

echo '

<form method="post" action="' . $PHP_SELF . '">
<font size="-1">
Match: <select name="method">'

. DisplayOption("and",     "All",     $method)
. DisplayOption("or",      "Any",     $method)
. DisplayOption("boolean", "Boolean", $method)
.

'</select>
Format: <select name="format">'

. DisplayOption("long",  "Long",  $format)
. DisplayOption("short", "Short", $format)
.

'</select>
Sort by: <select name="sort">'

. DisplayOption("score",    "Score",         $sort)
. DisplayOption("time",     "Time",          $sort)
. DisplayOption("title",    "Title",         $sort)
. DisplayOption("revscore", "Reverse Score", $sort)
. DisplayOption("revtime",  "Reverse Time",  $sort)
. DisplayOption("revtitle", "Reverse Title", $sort)

.
'</select>
</font>
<input type="hidden" name="config"   value="htdig">
<input type="hidden" name="restrict" value="">
<input type="hidden" name="exclude"  value="">
<input type="hidden" name="submit"   value="">

<SELECT NAME="matchesperpage">'
. DisplayOption("10",	"10 results",  $matchesperpage)
. DisplayOption("20",	"20 results",  $matchesperpage)
. DisplayOption("30",	"30 results",  $matchesperpage)
. DisplayOption("50",	"50 results",  $matchesperpage)
. DisplayOption("100",	"100 results", $matchesperpage)
.
'</select>

<br>
Search:
<input type="text" size="30" name="words" value="'. htmlspecialchars(StripSlashes($words))  . '">
&nbsp;<input type="submit" value="Search">
</form>
';
?>
