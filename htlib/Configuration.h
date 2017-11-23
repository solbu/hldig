//
// Configuration.h
//
// NAME
//
// reads the configuration file and manages it in memory.
//
// SYNOPSIS
//
// #include <Configuration.h>
//
// Configuration config;
//
// ConfigDefault config_defaults = {
//   { "verbose", "true" },
//   { 0, 0 }
// };
//
// config.Defaults(config_defaults);
//
// config.Read("~/.myconfig") ;
//
// config.Add("sync", "false");
//
// if(config["sync"]) ...
// if(config.Value("rate") < 50) ...
// if(config.Boolean("sync")) ...
//
// DESCRIPTION
//
// The primary purpose of the <b>Configuration</b> class is to parse
// a configuration file and allow the application to modify the internal
// data structure. All values are strings and are converted by the
// appropriate accessors. For instance the <b>Boolean</b> method will
// return numerical true (not zero) if the string either contains
// a number that is different from zero or the string <i>true</i>.
//
// The <i>ConfigDefaults</i> type is a structure of two char pointers:
// the name of the configuration attribute and it's value. The end of
// the array is the first entry that contains a null pointer instead of
// the attribute name. Numerical
// values must be in strings. For instance:
// <pre>
// ConfigDefault* config_defaults = {
//   { "wordlist_compress", "true" },
//   { "wordlist_page_size", "8192" },
//   { 0, 0 }
// };
// </pre>
// Returns the configuration (object of type <i>Configuration</i>)
// built if a file was found or config_defaults
// provided, 0 otherwise.
// The additional
// fields of the <b>ConfigDefault</b> are purely informative.
//
// FILE FORMAT
//
// This configuration file is a plain ASCII text file. Each line in
// the file is either a comment or contains an attribute.
// Comment lines are blank lines or lines that start with a '#'.
// Attributes consist of a variable name and an associated
// value:
//
// <pre>
// &lt;name&gt;:&lt;whitespace&gt;&lt;value&gt;&lt;newline&gt;
// </pre>
//
// The &lt;name&gt; contains any alphanumeric character or
// underline (_) The &lt;value&gt; can include any character
// except newline. It also cannot start with spaces or tabs since
// those are considered part of the whitespace after the colon. It
// is important to keep in mind that any trailing spaces or tabs
// will be included.
//
// It is possible to split the &lt;value&gt; across several
// lines of the configuration file by ending each line with a
// backslash (\). The effect on the value is that a space is
// added where the line split occurs.
//
// A configuration file can include another file, by using the special
// &lt;name&gt;, <tt>include</tt>. The &lt;value&gt; is taken as
// the file name of another configuration file to be read in at
// this point. If the given file name is not fully qualified, it is
// taken relative to the directory in which the current configuration
// file is found. Variable expansion is permitted in the file name.
// Multiple include statements, and nested includes are also permitted.
//
// <pre>
// include: common.conf
// </pre>
//
//
// END
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Configuration.h,v 1.11 2004/05/28 13:15:20 lha Exp $
//

#ifndef  _Configuration_h_
#define  _Configuration_h_

#include "Dictionary.h"
#include "htString.h"

struct ConfigDefaults
{
  const char *name;             // Name of the attribute
  /* FIXME: If I'm not mistaken, sometimes "value" is a constant and sometimes
   * it is not. If it's declared with "const" the build will fail. As it is now,
   * we get repeated warnings such as "deprecated conversion from
   * string constant to 'char*' [-Wwrite-strings] but the build completes.
   */
  char *value;                  // Default value
  const char *type;             // Type of the value (string, integer, boolean)
  const char *programs;         // Whitespace separated list of programs/modules using this attribute
  const char *block;            // Configuration block this can be used in (can be blank)
  const char *version;          // Version that introduced the attribute
  const char *category;         // Attribute category (to split documentation)
  const char *example;          // Example usage of the attribute (HTML)
  const char *description;      // Long description of the attribute (HTML)
};


class Configuration:public Object
{
public:
  //-
  // Constructor
  //
  Configuration ();
#ifndef SWIG
  Configuration (const Configuration & config):dcGlobalVars (config.
                                                             dcGlobalVars),
    separators (config.separators)
  {
    allow_multiple = config.allow_multiple;
  }
#endif                          /* SWIG */
  //-
  // Destructor
  //
   ~Configuration ()
  {
  }

  //
  // Adding and deleting items to and from the Configuration
  //
#ifndef SWIG
  //-
  // Add configuration item <b>str</b> to the configuration. The value
  // associated with it is undefined.
  //
  void Add (const String & str);
#endif /* SWIG */
  //-
  // Add configuration item <b>name</b> to the configuration and associate
  // it with <b>value</b>.
  //
  void Add (const String & name, const String & value);
  void AddParsed (const String & name, const String & value);
  //-
  // Remove the <b>name</b> from the configuration.
  //
  int Remove (const String & name);

  //-
  // Let the Configuration know how to parse name value pairs.
  // Each character of string <b>s</b> is a valid separator between
  // the <i>name</i> and the <i>value.</i>
  //
  void NameValueSeparators (const String & s);

  //-
  // Read name/value configuration pairs from the file <b>filename</b>.
  //
  virtual int Read (const String & filename);

  //-
  // Return the value of configuration attribute <b>name</b> as a
  // <i>String</i>.
  //
  const String Find (const String & name) const;

  //-
  // Return 1 if the value of configuration attribute <b>name</b> has
  // been set, 0 otherwise
  int Exists (const String & name) const;

#ifndef SWIG
  //-
  // Alias to the <b>Find</b> method.
  //
  const String operator[] (const String & name) const;
#endif /* SWIG */
  //-
  // Return the value associated with the configuration attribute
  // <b>name</b>, converted to integer using the atoi(3) function.
  // If the attribute is not found in the configuration and
  // a <b>default_value</b> is provided, return it.
  //
  int Value (const String & name, int default_value = 0) const;
  //-
  // Return the value associated with the configuration attribute
  // <b>name</b>, converted to double using the atof(3) function.
  // If the attribute is not found in the configuration and
  // a <b>default_value</b> is provided, return it.
  //
  double Double (const String & name, double default_value = 0) const;
  //-
  // Return 1 if the value associated to <b>name</b> is
  // either <b>1, yes</b> or <b>true</b>.
  // Return 0 if the value associated to <b>name</b> is
  // either <b>0, no</b> or <b>false</b>.
  //
  int Boolean (const String & name, int default_value = 0) const;
  Object *Get_Object (char *name);

  //-
  // Load configuration attributes from the <i>name</i> and <i>value</i>
  // members of the <b>array</b> argument.
  //
  void Defaults (const ConfigDefaults * array);

protected:
  Dictionary dcGlobalVars;
  String separators;
  int allow_multiple;
};

#endif
