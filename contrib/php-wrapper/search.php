<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2//EN">
<?
   # $Id: search.php,v 1.1 2001/08/17 19:32:21 junkmale Exp $
   #
   # This is a php wrapper for use with htdig (http://www.htdig.org/)
   #
   # Copyright (c) 2001 DVL Software Limited
   # http://www.dvl-software.com/
   #

   # This is basically a BSD License.  I got this from 
   # http://www.FreeBSD.org/copyright/freebsd-license.html
   #
   # Redistribution and use in source and binary forms, with or without 
   # modification, are permitted provided that the following conditions are met: 
   #
   #   1.Redistributions of source code must retain the above copyright notice, 
   #     this list of conditions and the following disclaimer. 
   #   2.Redistributions in binary form must reproduce the above copyright notice, 
   #     this list of conditions and the following disclaimer in the documentation 
   #     and/or other materials provided with the distribution. 

   # Items which are often configurable
   #
   $Debug = 0;  # set to non-zero to display debugging messages

   # change this to the location of the shell script call to htsearch
   #
   $HTSEARCH_PROG = "/www/freebsddiary/htdig/htdig.sh";


   #
   # end of items which are often configurable

   #
   # these are the variables from the form. populated only
   # if the user clicked on Search
   #
   $Parameters = $HTTP_POST_VARS;

   #
   # these are the variables within the URL.  These are populated
   # on pages after the first page.  we perform this step to ensure
   # the form is populated with the appropriate values
   #
   $QueryString = $HTTP_SERVER_VARS["QUERY_STRING"];
   if ($QueryString) {
      $ArrayParm  = ConvertQueryStringToArray($QueryString, ";");

      #
      # these are the fields which the user fills in
      #
      $method   = $ArrayParm["method"];
      $format   = $ArrayParm["format"];
      $sort     = $ArrayParm["sort"];
      $words    = urldecode($ArrayParm["words"]);

      # these fields are hidden and therefore don't need to be populated
      # but are provided for completeness

      #$config   = $ArrayParm["config"];
      #$restrict = $ArrayParm["restrict"];
      #$exclude  = $ArrayParm["exclude"];
      #$submit   = $ArrayParm["submit"];
      #$page     = $ArrayParm["page"];
   }

?>
<html>

<head>
<title>Sample htdig php interface</title>
</head>

<body bgcolor="#ffffff" link="#0000cc">
<h3>Testing php wrapper for htdig.</h3>
<?
if (!$Parameters && !strlen($HTTP_SERVER_VARS["QUERY_STRING"])) {
?>
<pre>
#
# htdig php wrapper 1.0
#
# Copyright (c) 2001 DVL Software Limited
# http://www.dvl-software.com/
#
# this source code can be obtained from:
#      http://freebsddiary.org/samples/htdig-php-wrapper.tar.gz
#
# If you have any trouble with htdig, try the htdig mailing lists at
# http://www.htdig.org/
#
# If you have any suggestions for improvments, bug reports, etc,
# please send patches to me.  Thanks.
#
# I'd also like to hear about where it's being used.  That's just me
# being curious, that's all.
#
# dan@langille.org
#
</pre>
<?
}
?>

Please try to do searches which bring back multiple pages of results and then<br>
click through to subsequent pages of the results.  Try various options.  Basically,<br>	
try to break the search. This code will be released to the htdig project. Thanks.


<?
   require("search-form.php");

function ConvertQueryStringToArray($QueryString, $Delimiter) {
   # this function takes a string which contains parameters.  The parameters are delimited
   # by $Delimiter.  It splits these parameters up into an array.  It then takes the key/value
   # pairs of this array and puts it into an associative array.
   #
   # for example, if the input is: $QueryString = "size=10;colour=L;fabric=cotton"
   #                               $Delimiter   = ";"
   #
   # then the output will be this array:
   #
   #   array (
   #       "size"   => "10",
   #       "colour" => "L",
   #       "fabric" => "cotton"
   #   );
   #
   # It is assumed that the string is of the format: "key1=value1<*>key2=value2<*>key3=value3..."
   # where <*> is $Delimiter.
   #
   # This function returns an empty variable if no parameters are found.

#   echo "ConvertParametersToArray: QueryString = '$QueryString' with length " . strlen($QueryString) . "<br>\n";

   #
   # if there's nothing to do, do nothing.
   #
   if (strlen($QueryString)) {

      #
      # split the query string into an array.
      #
      $SimpleArray = explode($Delimiter, $QueryString);

      #
      # taken each element of the array, which will have
      # elements 0..n where each element is of the form
      # "keyn"="valuen" and split them into "keyn" and "valuen"
      #
      while (list($key, $value) = each($SimpleArray)) {
         list($KeyN, $ValueN) = split("=", $value);
#         echo "key=$KeyN";
         #
         # put that key/value pair into the result we are going to pass back
         #
         $Result[$KeyN] = $ValueN;
      }
   } else {
      echo "nothing found<br>\n";
   }

   return $Result;
}

function CompileQuery($HTTP_POST_VARS) {

   $query = '';

   while (list($name, $value) = each($HTTP_POST_VARS)) {
      $query = $query . "$name=$value;";
   }

   # remove the trailing ;
   $query = substr($query, 0, strlen($query) - 1);

   return $query;
}

#
# if the user clicked on Search or we have a query string
#
if ($Parameters || strlen($HTTP_SERVER_VARS["QUERY_STRING"])) {

   if (count($Parameters)) {
      $query = CompileQuery($Parameters);
   } else {
      $query = $HTTP_SERVER_VARS["QUERY_STRING"];
   }

   #
   # this code courtesy of an article by Colin Viebrock [colin@easyDNS.com]
   # at http://www.devshed.com/Server_Side/PHP/search/
   # which formed the basis of this work
   #
   #
   # execute the htsearch code
   #
   $command="$HTSEARCH_PROG \"$query\"";
   exec($command,$result);

   # debug: look at the output.  useful for seeing what is where
   if ($Debug) {
      while (list($k,$v) = each($result)) {
         echo "$k -> $v \n<BR>";
      }
   }

   # how many rows do we have?
   $rc = count($result);

   # all these magic numbers have got to go
   if ($rc < 3) {
      echo "There was an error executing this query.  Please try later.\n";
   } else {
      if ($result[2]=="NOMATCH") {
         echo "There were no matches for <B>$words</B> found on the website.<P>\n";
      } else {
         if ($result[2]=="SYNTAXERROR") {
            echo "There is a syntax error in your search for <B>$search</B>:<BR>";
            echo "<PRE>" . $result[3] . "</PRE>\n";
         } else {
            #
            # display the headers, this includes the forum, the search
            # parameters, etc.
            #

            $ResultSetStart = 1;
            for ($i = $ResultSetStart; $i < $rc; $i++) {
               echo $result[$i];
            }
         }
      }
   }
}
?>

</body>
</html>

