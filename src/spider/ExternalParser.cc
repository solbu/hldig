//
// ExternalParser.cc
//
// ExternalParser: Implementation of ExternalParser
//                 Allows external programs to parse unknown document formats.
//                 The parser is expected to return the document in a 
//                 specific format. The format is documented 
//                 in http://www.htdig.org/attrs.html#external_parser
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: ExternalParser.cc,v 1.1.2.2 2007/04/26 16:59:35 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "ExternalParser.h"
//#include "HTML.h"
//#include "Plaintext.h"
//#include "htdig.h"
#include "htString.h"
#include "QuotedStringList.h"
#include "URL.h"
#include "Dictionary.h"
#include "good_strtok.h"

#include "HtDebug.h"

#include <ctype.h>
#include <stdio.h>

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif

#include <stdlib.h>
#ifdef HAVE_WAIT_H
#include <wait.h>
#elif HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef _MSC_VER /* _WIN32 */
#include <process.h>
#endif


#include "defaults.h"

static Dictionary	*parsers = 0;
static Dictionary	*toTypes = 0;
extern String		configFile;

//*****************************************************************************
// ExternalParser::ExternalParser(char *contentType)
//
ExternalParser::ExternalParser(char *contentType)
{
    String mime;
    int sep;

    contents = 0;

    if (canParse(contentType))
    {
        String mime = contentType;
        mime.lowercase();
        sep = mime.indexOf(';');
        if (sep != -1)
            mime = mime.sub(0, sep).get();

        currentParser = ((String *)parsers->Find(mime))->get();
    }
    ExternalParser::contentType = contentType;
}


//*****************************************************************************
// ExternalParser::~ExternalParser()
//
ExternalParser::~ExternalParser()
{
}


//*****************************************************************************
// int ExternalParser::readLine(FILE *in, String &line)
//
int
ExternalParser::readLine(FILE *in, String &line)
{
    char	buffer[2048];
    int		length;
    
    line = 0; // read(in, buffer, sizeof(buffer)
    while (fgets(buffer, sizeof(buffer), in))
    {
	length = strlen(buffer);
	if (buffer[length - 1] == '\n')
	{
	    //
	    // A full line has been read.  Return it.
	    //
	    line << buffer;
	    line.chop('\n');
	    return 1;
	}
	else
	{
	    //
	    // Only a partial line was read.  Append it to the line
	    // and read some more.
	    //
	    line << buffer;
	}
    }
    return line.length() > 0;
}


//*****************************************************************************
// int ExternalParser::canParse(char *contentType)
//
int
ExternalParser::canParse(char *contentType)
{
    HtConfiguration* config= HtConfiguration::config();
    int			sep;

    if (!parsers)
    {
        parsers = new Dictionary();
        toTypes = new Dictionary();

        QuotedStringList	qsl(config->Find("external_parsers"), " \t");
        String			from, to;
        int			i;

        for (i = 0; qsl[i]; i += 2)
        {
            from = qsl[i];
            to = "";
            sep = from.indexOf("->");
            if (sep != -1)
            {
                to = from.sub(sep+2).get();
                from = from.sub(0, sep).get();
            }
            from.lowercase();
            sep = from.indexOf(';');
            if (sep != -1)
                from = from.sub(0, sep).get();

            parsers->Add(from, new String(qsl[i + 1]));
            toTypes->Add(from, new String(to));
        }
    }

    String mime = contentType;
    mime.lowercase();
    sep = mime.indexOf(';');
    if (sep != -1)
        mime = mime.sub(0, sep).get();
    return parsers->Exists(mime);
}

//*****************************************************************************
// void ExternalParser::externalParse()
//
char * ExternalParser::externalParse(URL &base)
{
// NEAL - ENABLE/REWRITE THIS ASAP FOR WIN32
#ifndef _MSC_VER /* _WIN32 */
    HtConfiguration * config= HtConfiguration::config();
    HtDebug * debug = HtDebug::Instance();
    if (contents == 0 || contents->length() == 0 || currentParser.length() == 0)
    {
        return NULL;
    }

    //
    // Write the contents to a temporary file.
    //
    String      path = getenv("TMPDIR");
    int		fd;
    if (path.length() == 0)
        path = "/tmp";
#ifndef HAVE_MKSTEMP
    path << "/htdext." << getpid(); // This is unfortunately predictable

#ifdef O_BINARY
    fd = open((char*)path, O_WRONLY|O_CREAT|O_EXCL|O_BINARY);
#else
    fd = open((char*)path, O_WRONLY|O_CREAT|O_EXCL);
#endif
#else
    path << "/htdex.XXXXXX";
    fd = mkstemp((char*)path);
    // can we force binary mode somehow under Cygwin, if it has mkstemp?
#endif
    if (fd < 0)
    {
        debug->outlog(0, "External parser error: Can't create temp file %s\n", (char *)path);
        return NULL;
    }

    write(fd, contents->get(), contents->length());
    close(fd);

    //  unsigned int minimum_word_length = config->Value("minimum_word_length", 3);
    String	line;
    char    *token1;
    //int		loc = 0, hd = 0;
    URL		url;
    String mime = contentType;
    mime.lowercase();
    int	sep = mime.indexOf(';');
    if (sep != -1)
        mime = mime.sub(0, sep).get();
    String	convertToType = ((String *)toTypes->Find(mime))->get();
    if (convertToType.length() == 0)
    {
        debug->outlog(0, "External parser error: nothing to convert to from type %s\n", contentType.get());
        return NULL;
    }
    int		get_hdr = (convertToType.nocase_compare("user-defined") == 0);
    String	newcontent;

    StringList	cpargs(currentParser, " \t");
    char   **parsargs = new char * [cpargs.Count() + 5];
    int    argi;
    for (argi = 0; argi < cpargs.Count(); argi++)
        parsargs[argi] = (char *)cpargs[argi];
    parsargs[argi++] = path.get();
    parsargs[argi++] = contentType.get();
    parsargs[argi++] = (char *)base.get().get();
    parsargs[argi++] = configFile.get();
    parsargs[argi++] = 0;

    int    stdout_pipe[2];
    int	   fork_result = -1;
    int	   fork_try;

    if (pipe(stdout_pipe) == -1)
    {
        debug->outlog(0, "External parser error: Can't create pipe!\n");
        unlink((char*)path);
        delete [] parsargs;
        return NULL;
    }

    for (fork_try = 4; --fork_try >= 0;)
    {
        fork_result = fork(); // Fork so we can execute in the child process
        if (fork_result != -1)
            break;
        if (fork_try)
            sleep(3);
    }
    if (fork_result == -1)
    {
        debug->outlog(0, "Fork Failure in ExternalParser\n");
        unlink((char*)path);
        delete [] parsargs;
        return NULL;
    }

    if (fork_result == 0) // Child process
    {
        close(STDOUT_FILENO); // Close handle STDOUT to replace with pipe
        dup(stdout_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(STDIN_FILENO); // Close STDIN to replace with file
        open((char*)path, O_RDONLY);

        // Call External Parser
        execv(parsargs[0], parsargs);

        // uh oh
        perror("execv");
        write(STDERR_FILENO, "External parser error: Can't execute ", 37);
        write(STDERR_FILENO, parsargs[0], strlen(parsargs[0]));
        write(STDERR_FILENO, "\n", 1);
        _exit(EXIT_FAILURE);
    }

    // Parent Process
    delete [] parsargs;
    close(stdout_pipe[1]); // Close STDOUT for writing
#ifdef O_BINARY
    FILE *input = fdopen(stdout_pipe[0], "rb");
#else
    FILE *input = fdopen(stdout_pipe[0], "r");
#endif
    if (input == NULL)
    {
        debug->outlog(0, "Fdopen Failure in ExternalParser\n");
        unlink((char*)path);
        return NULL;
    }

    //
    // if the mime type is 'user-defined', then the REAL mime type
    // should be the first line of the output
    //
    if (get_hdr)
    {
        readLine(input, line);
        line.chop('\r');
        if (line.length() == 0)
        {
            debug->outlog(0, "Header information missing in ExternalParser\n");
            return NULL;
        }
        else if (mystrncasecmp((char*)line, "content-type:", 13) == 0)
        {
            token1 = line.get() + 13;
            while (*token1 && isspace(*token1))
                token1++;
            token1 = strtok(token1, "\n\t");
            convertToType = token1;
        }
    }

    if (!canParse(convertToType) && mystrncasecmp((char*)convertToType, "text/", 5) != 0)
    {
        if (mystrcasecmp((char*)convertToType, "user-defined") == 0)
            cerr << "External parser error: no Content-Type given\n";
        else
            cerr << "External parser error: can't parse Content-Type \""
                << convertToType << "\"\n";
        cerr << " URL: " << base.get() << "\n";

        return NULL;
    }
    else
    {
        char    buffer[2048];
        int     length;
        int     nbytes = config->Value("max_doc_size");
        while (nbytes > 0 && (length = fread(buffer, 1, sizeof(buffer), input)) > 0)
        {
            nbytes -= length;
            if (nbytes < 0)
                length += nbytes;
            newcontent.append(buffer, length);
        }
    }
    fclose(input);
    // close(stdout_pipe[0]); // This is closed for us by the fclose()

    int rpid, status;
    while ((rpid = wait(&status)) != fork_result && rpid != -1)
        ;
    unlink((char*)path);

    if (newcontent.length() > 0)
    {
        //static HTML			*html = 0;
        //static Plaintext		*plaintext = 0;
        //Parsable			*parsable = 0;

        contentType = convertToType;
        if (canParse(contentType))
        {
            currentParser = ((String *)parsers->Find(contentType))->get();
            setContents(newcontent.get(), newcontent.length());
            return externalParse(base);
        }
        else if ((mystrncasecmp((char*)contentType, "text/html", 9) == 0)
            || (mystrncasecmp((char*)contentType, "text/plain", 10) == 0))
        {
            char * output = (char *)malloc(sizeof(char) * newcontent.length() + 1);
            memcpy(output, newcontent.get(), newcontent.length());
            output[newcontent.length()] = '\0';

            return output;
        }
        else 
        {
            //
            // can't parse with an external parser, and isn't just HTML or
            // plain text, so give up 
            //
            return NULL;
        }
    }
    else
    {
        //
        // coudn't extract any text... give up
        //
        return NULL;
    }
#endif //ifndef _MSC_VER /* _WIN32 */
}


void ExternalParser::setContents(char *data, int length)
{
    if (contents)
    {
        delete contents;
    }
    contents = new String(data, length);
}

