/* RTF2HTML.c, Chuck Shotton - 6/21/93 */
/************************************************************************
 * This program takes a stab at converting RTF (Rich Text Format) files
 * into HTML. There are some limitations that keep RTF from being able to
 * easily represent things like in-line images and anchors as styles. In
 * particular, RTF styles apply to entire "paragraphs", so anchors or
 * images in the middle of a text stream can't easily be represented by
 * styles. The intent is to ultimately use something like embedded text
 * color changes to represent these constructs. 
 * 
 * In the meantime, you can take existing Word documents, apply the
 * correct style sheet, and convert them to HTML with this tool.
 *
 * AUTHOR: Chuck Shotton, UT-Houston Academic Computing,
 *         cshotton@oac.hsc.uth.tmc.edu
 *         
 *         Dmitry Potapov, CapitalSoft
 *         dpotapov@capitalsoft.com
 *
 *         David Lippi, Comune di Prato, Italy
 *         d.lippi@comune.prato.it
 *
 *         Gabriele Bartolini, Comune di Prato, Italy
 *         g.bartolini@comune.prato.it
 *
 * USAGE: rtf2html [rtf_filename] 
 *
 * BEHAVIOR:
 *        rtf2html will open the specified RTF input file or read from
 *        standard input, writing converted HTML to standard output.
 *
 * NOTES:
 *        The RTF document must be formatted with a style sheet that has
 *        style numberings that conform to the style_mappings table
 *        defined in this source file. Characters are converted according
 *        to the ANSI Windows 1252 code or Macintosh.
 *
 * MODIFICATIONS:
 *         6/21/93 : Chuck Shotton - created version 1.0.
 *        11/26/98 : Dmitry Potapov - version 1.1 beta
 *        05/07/04 : David Lippi, Gabriele Bartolini - version 1.2
 *
 * Copyright (C) 2004 Comune di Prato
 * 
 * For copyright details, see the file COPYING in your distribution
 * or the GNU General Public License (GPL) version 2 or later
 * <http://www.gnu.org/copyleft/gpl.html>
 *
 ************************************************************************/

/* Note, the source is formated with 4 character tabs */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "charset1252.h"
#include "charsetmac.h"

#ifdef _MSC_VER
#  define  strcasecmp _stricmp
#endif

#ifndef TRUE
#define TRUE -1
#define FALSE 0
#endif

#define MAX_LEVELS 40  /*defines the # of nested in-line styles (pairs of {})*/
#define MAX_RTF_TOKEN 40

#define MAX_INLINE_STYLES 5 /*defines # of in-line styles, bold, italic, etc.*/

typedef struct tag_StyleState
{
  unsigned char s: MAX_INLINE_STYLES;
} TStyleState;

typedef enum { s_plain, s_bold, s_italic, s_underline, s_hidden, /*in-line styles*/
  s_para,  s_br,    /*pseudo style*/
  s_h0, s_h1, s_h2, s_h3, s_h4, s_h5, s_h6 /*heading styles*/
} StyleState;

char *styles[][2] = {    /*HTML Start and end tags for styles*/
  {"", ""},
  {"<strong>", "</strong>"},
  {"<em>", "</em>"},
  {"", ""},
  {"<!-- ", " -->"},
  {"\n", "\n"}, /* {"\n<p>", "</p>\n"}, */
  {"<br />\n",""},
  {"", ""},
  {"<h1>", "</h1>"},
  {"<h2>", "</h2>"},
  {"<h3>", "</h3>"},
  {"<h4>", "</h4>"},
  {"<h5>", "</h5>"},
  {"<h6>", "</h6>"}
};

/* style_mappings maps the style numbers in a RTF style sheet into one of the*/
/* (currently) six paragraph-oriented HTML styles (i.e. heading 1 through 6.)*/
/* Additional styles for lists, etc. should be added here. Style info        */
/* ultimately should be read from some sort of config file into these tables.*/

#define MAX_NAME_LEN 40
char style_name[MAX_NAME_LEN];

#define STYLE_NUMBER 7
char *style_namings[STYLE_NUMBER] = {
  "", "heading 1", "heading 2", "heading 3", "heading 4", "heading 5",
  "heading 6"
};
char style_mappings[STYLE_NUMBER][MAX_RTF_TOKEN];
char style_number[MAX_RTF_TOKEN];

/* RTF tokens that mean something to the parser. All others are ignored. */

typedef enum {
  t_start,
  t_fonttbl, t_colortbl, t_stylesheet, t_info, t_s, t_b, t_ul, t_ulw, 
  t_uld, t_uldb, t_i, t_v, t_plain, t_par, t_pict, t_tab, t_bullet, 
  t_cell, t_row, t_line, t_endash, t_emdash, t_rquote,
  t_end
} TokenIndex;

char *tokens[] = {
  "###", 
  "fonttbl", "colortbl", "stylesheet", "info", "s", "b", "ul", "ulw", 
  "uld", "uldb", "i", "v", "plain", "par", "pict", "tab", "bullet",
  "cell", "row", "line", "endash", "emdash", "rquote",
  "###"
};

TStyleState style_state[MAX_LEVELS], curr_style;
short curr_heading;

void (*RTF_DoControl)(FILE*,char*,char*);
char isBody;
char* title;
//FILE* f;

short   level,    /*current {} nesting level*/ 
  skip_to_level,/*{} level to which parsing should skip (used to skip */ 
                /*  font tables, style sheets, color tables, etc.)    */ 
  gobble,  /*Flag set to indicate all input should be discarded  */ 
  ignore_styles;/*Set to ignore inline style expansions after style use*/

/* Charset */
unsigned char** charset_table;

#define CHARSET_DEFAULT 0 // Index of the default charset to use
#define CHARSET_NUMBER 2 // Number of charset used
#define CHARSET_MAX_LENGTH 20 // Max numbero of char in the charset
// metadata used in rtf standard for the  charset definition
unsigned char *charset[CHARSET_NUMBER] = {
  "ansi", 
  "mac"
};
// variable with the charset definition
unsigned char **charset_variablename[CHARSET_NUMBER] = {
  charset1252, 
  mac
};

/**************************************/

int openfile (char * filename, FILE ** f)
{
  int rv = 1;

  if (filename) 
  {
    if (!(*f = fopen (filename, "r"))) 
    {
      fprintf (stderr, "\nError: Input file %s not found.\n", filename);
      rv = 0;
    }
    else
    {
      title = filename;
    }
  } 
  else 
  {
    *f = stdin;
    title="STDIN";
  }
  return rv;
}

/**************************************/

int closefile (FILE * f)
{
  return fclose (f);
}

/**************************************/

char RTF_GetChar( FILE* f )
{
  char ch;
  do
  {
    ch = fgetc( f );
  } while ((ch=='\r')||(ch=='\n'));
  return ch;
}

/**************************************/

char RTF_UnGetChar(FILE* f, char ch)
{
  return ungetc(ch, f);
}

/**************************************/

void RTF_PutStr(char* s)
{
  if (gobble) return;
  fputs(s, stdout);
}

/**************************************/

void RTF_PutHeader()
{
  RTF_PutStr("<head>\n<title>");
  RTF_PutStr(title);
  RTF_PutStr("</title>\n");
  RTF_PutStr("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n");
  RTF_PutStr("</head>\n");
}

/**************************************/

void RTF_PutChar(char ch)
{
  if (gobble) return;
  if (!isBody)
  {
    RTF_PutHeader();
    RTF_PutStr("<body>\n");
    isBody=TRUE;
  }
  switch (ch) {
    case '<':
      RTF_PutStr("&lt;");
      break;
      
    case '>':
      RTF_PutStr("&gt;");
      break;
      
    case '&':
      RTF_PutStr("&amp;");
      break;
    
    default:
      fputc(ch, stdout);
  }
}

/**************************************/

void RTF_PlainStyle (TStyleState* s)
{
  int i;
  for(i=0;i<MAX_INLINE_STYLES;i++)
  {
    if(s->s & (1<<i))
      RTF_PutStr(styles[i][1]);
  }
  s->s=0;
}

/**************************************/

void RTF_SetStyle(TStyleState* s, StyleState style)
{
  if( (!ignore_styles||(style==s_hidden)) && ((s->s&(1<<style))==0) )
  {
    RTF_PutStr(styles[style][0]);
    s->s|=(1<<style);
  }
}

/**************************************/

void RTF_PushState(short* level)
{
  if(*level>=MAX_LEVELS)
  {
    fprintf(stderr,"Exceed maximum level\n");
    exit(-1);
  }
  style_state[*level]=curr_style;
  (*level)++;
}

/**************************************/

void RTF_PopState(short* level)
{
  int j;
  TStyleState new_style;
  
  if(*level<1)
  {
    fprintf(stderr,"RTF parse error: unexpected '}'\n");
    exit(-1);
  }
  new_style = style_state[*level-1];
  /*close off any in-line styles*/
  for (j=0;j<MAX_INLINE_STYLES;j++) 
  {
    if ( ((curr_style.s & (1<<j))!=0) && ((new_style.s & (1<<j))==0) )
    {
      curr_style.s &= ~(1<<j);
      RTF_PutStr(styles[j][1]);
    }
  }
  
  for (j=0;j<MAX_INLINE_STYLES;j++) 
  {
    if( ((curr_style.s & (1<<j))==0) && ((new_style.s & (1<<j))!=0) )
      RTF_PutStr(styles[j][0]);
  }
  (*level)--;
  curr_style = new_style;

  if (*level == skip_to_level) {
    skip_to_level = -1;
    gobble = FALSE;
  }
}

/**************************************/
/* Map a style number into a HTML heading */

short RTF_MapStyle(char* s)
{
  int i;
  for (i=0;i<7;i++)
    if (!strcmp(style_mappings[i], s))
      return (i);
  return (0);
}

/**************************************/

void RTF_AddStyleMap(char* name, char* number)
{
  int i, len;
  len=strlen(name);
  if( name[len-1]==';') name[--len]=0;
  for(i=0;i<STYLE_NUMBER;i++)
  {
    if(!strcasecmp(name,style_namings[i]))
    {
      strcpy(style_mappings[i],number);
      return;
    }
  }
}

/**************************************/

void RTF_BuildName(char* token, char* ch, unsigned is_string)
{
  int len;
  char *p;
  len = strlen(token);
  if(len>=MAX_NAME_LEN-1)
    return;
  if (is_string)
  {
    for (p = ch; p && *p; ++p)
    {
      token[len]=*p;
      ++len;
    }
  }
  else
  {
    token[len] = *ch;
    ++len;
  }
  token[len]='\0';
}


/**************************************/

void RTF_ClearName(char* token)
{
  token[0]=0;
}

/**************************************/

TokenIndex GetTokenIndex(char* control)
{
  TokenIndex i;

  for (i=t_start; i<t_end; i++) 
  {
    if(control[0]==tokens[i][0]) /* Added for fast compare */
    {
      if (!strcmp(control, tokens[i]))
      {
        break;
      }
    }
  }
  return i;
}

/**************************************/

void RTF_DoStyleControl (FILE* f, char* control, char* arg)
{
  if(GetTokenIndex(control)==t_s)
  {
    strcpy(style_number,arg);
  }
}

/**************************************/

int chartoi(char ch)
{
  if((ch>='0')&&(ch<='9'))
    return ch-'0';
  if((ch>='A')&&(ch<='Z'))
    return ch-'A'+10;
  if((ch>='a')&&(ch<='z'))
    return ch-'a'+10;
  return -1;
}

/**************************************/

void RTF_BuildArg (FILE * f, char ch, char* arg)
{
  int i=0;

  if(feof(f))
  {
    arg[0]=0;
    return;
  }
  if(ch=='-')
  {
    arg[i++]='-';
    ch = RTF_GetChar( f );
    if(feof(f))
    {
      arg[0]=0;
      return;
    }
  }
  for(;isdigit(ch);i++)
  {
    arg[i]=ch;
    if(i>=MAX_RTF_TOKEN-1)
    {
      arg[MAX_RTF_TOKEN-1]=0;
      while(isdigit(ch)) {
        ch = RTF_GetChar( f );
        if(feof(f))
          return;
      } 
      break;
    }
    ch = RTF_GetChar( f );
    if(feof(f))
    {
      arg[i+1]=0;
      return;
    }
  }
  arg[i]=0;
  if(!isspace(ch))
  {
    RTF_UnGetChar(f, ch);
  }
}
  
/**************************************/

void RTF_BuildToken (FILE* f, char ch)
{
  int i;
  
  for(i=1;;i++)
  {
    char token[MAX_RTF_TOKEN], arg[MAX_RTF_TOKEN];
    token[i-1]=ch;
    if(i>=MAX_RTF_TOKEN-1)
    {
      do {
        ch = RTF_GetChar( f );
        if(feof(f))
          return;
      } while (isalpha(ch));   
      RTF_BuildArg(f, ch,arg);
      return;
    }
    ch = RTF_GetChar( f );
    if(feof(f))
    {
      token[i]=0;
      RTF_DoControl(f,token,"");
      return;
    }
    if( !isalpha(ch) )
    {
      token[i]=0;
      RTF_BuildArg(f, ch,arg);
      RTF_DoControl(f,token,arg);
      return;
    }
  }
}

/**************************************/

void RTF_backslash(FILE* f, char** pch, char* pf)
{
  int ch;
  *pf=FALSE;
  ch = RTF_GetChar( f );
  if(feof(f))
  {
    fprintf(stderr,"Unexpected end of file\n");
    return;
  }
  switch (ch) 
  {
    case '\\':
      *pch=charset_table[92]; *pf=TRUE;
      break;
    case '{':
      *pch=charset_table[123]; *pf=TRUE;
      break;
    case '}':
      *pch=charset_table[125]; *pf=TRUE;
      break;
    case '*':
      gobble = TRUE;  /*perform no output, ignore commands 'til level-1*/
      if(skip_to_level>level-1||skip_to_level==-1) 
        skip_to_level = level-1;
      break;
    case '\'':
    {
      char ch1, ch2;
      ch1 = RTF_GetChar( f );
      ch2 = RTF_GetChar( f );
      if(!feof(f)) 
      {
        if(isxdigit(ch1)&&isxdigit(ch2))
        {
          ch = chartoi(ch1)*16+chartoi(ch2);
          *pch = charset_table[ch-1]; *pf=TRUE;
        } else {
          fprintf(stderr,"RTF Error: unexpected '%c%c' after \\\'\n",ch1,ch2);
        }
      }
      break;
    }
    default:
      if (isalpha(ch)) 
      {
        RTF_BuildToken(f, ch);
      } else {
        fprintf(stderr, "\nRTF Error: unexpected '%c' after \\.\n", ch);
      }
      break;
  }
}

/**************************************/

void RTF_ParseStyle(FILE * f)
{
  char ch, pf;
  char *code;
  int level0;
  void (*PrevDoControl)(FILE*,char*,char*);
  
  level0=level;
  PrevDoControl=RTF_DoControl;
  RTF_DoControl=RTF_DoStyleControl;
  
  RTF_ClearName(style_name);
  style_number[0]=0;
  while (1) 
  {
    ch = RTF_GetChar( f );
    if(feof(f))
      break;
    switch (ch) 
    {
      case '\\':
        RTF_backslash(f, &code, &pf);
        if(pf)
        {
          RTF_BuildName(style_name, code, 1);
        } else {
          RTF_ClearName(style_name);
        }
        break;
      
      case '{':
        level++;
        RTF_ClearName(style_name);
        break;
      
      case '}':
        if(level0+1==level)
        {
          if(style_number[0]!=0)
          {
            RTF_AddStyleMap(style_name,style_number);
            style_number[0]=0;
          }
        } else if(level0==level) {
          RTF_DoControl=PrevDoControl;
          RTF_UnGetChar(f, ch);
          return;
        }
        level--;
        RTF_ClearName(style_name); 
        break;
        
      default:
        RTF_BuildName(style_name, &ch, 0);
        break;
    }
  } /* while */
}

/**************************************/
/* Perform actions for RTF control words */

void RTF_DoBodyControl (FILE * f, char* control,char* arg)
{
  short style;

  if (gobble) return;

  switch (GetTokenIndex(control)) 
  {
    case t_stylesheet:
      gobble = TRUE;  /*perform no output, ignore commands 'til level-1*/
      skip_to_level = level-1;
      RTF_ParseStyle( f );
      break;
    case t_fonttbl:  /*skip all of these and their contents!*/
    case t_colortbl:
    case t_info:
      gobble = TRUE;  /*perform no output, ignore commands 'til level-1*/
      skip_to_level = level-1;
      break;
    case t_pict:
      gobble = TRUE;  /*perform no output, ignore commands 'til level-1*/
      if(skip_to_level>=level || skip_to_level==-1) 
        skip_to_level = level-1;
      break;
      
      
    case t_s: /*Style*/
      if (!curr_heading) 
      {
        style = RTF_MapStyle (arg);
        if(style)
        {
          curr_heading = s_h0 + style;
          RTF_PutStr(styles[curr_heading][0]);
          ignore_styles = TRUE;
        }
      }
      break;
      
    case t_b: /*Bold*/
        RTF_SetStyle(&curr_style,s_bold);
      break;
      
    case t_ulw:
    case t_uld:
    case t_uldb:
    case t_ul: /*Underline, maps to "emphasis" HTML style*/
        RTF_SetStyle(&curr_style,s_underline);
      break;
      
    case t_i: /*Italic*/
        RTF_SetStyle(&curr_style,s_italic);
      break;
      
    case t_v: /* Hidden*/
        RTF_SetStyle(&curr_style,s_hidden);
      break;
      
    case t_par: /*Paragraph*/
      if (curr_heading!=s_plain) {
        RTF_PutStr(styles[curr_heading][1]);
        curr_heading = s_plain;
      } else {
        RTF_PutStr(styles[s_para][0]);
      }
      ignore_styles = FALSE;
      break;
      
    case t_plain: /*reset inline styles*/
      RTF_PlainStyle(&curr_style);
      break;
    case t_cell:
    case t_tab:
      RTF_PutChar(' ');
      break;
    case t_endash:
    case t_emdash:
      RTF_PutChar('-');
      break;
    case t_line:
    case t_row:
      RTF_PutStr(styles[s_br][0]);
      break;
    case t_bullet:
      RTF_PutChar('\xb7');
      break;
    case t_start:
    case t_end:
      break;
    case t_rquote:
      //RTF_PutStr("&rsquo;");
      RTF_PutStr("'");
      break;
  }
      
}

/**************************************/
/* RTF_Parse is a crude, ugly state machine that understands enough of */
/* the RTF syntax to be dangerous.                                     */

void RTF_ParseBody( FILE* f )
{
  char ch, pf;
  char* code;

  RTF_DoControl=RTF_DoBodyControl;
  level = 0;
  skip_to_level = -1;
  gobble = FALSE;
  ignore_styles = FALSE;
  
  while (1) 
  {
    ch = RTF_GetChar( f );
    if (feof(f))
    {
      break;
    }
    switch (ch) 
    {
      case '\\':
        RTF_backslash(f, &code,&pf);
        if(pf && code)
          RTF_PutStr(code);
        break;
      
      case '{':
        RTF_PushState(&level);
        break;
      
      case '}':
        RTF_PopState(&level);
        break;
        
      default:
        RTF_PutChar(ch);
        break;
    }
  }/*while*/
}

/**************************************/

int RTF_Parse (FILE* f)
{
  RTF_PutStr("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML//EN\">\n<html>\n");

  isBody=FALSE;

  RTF_ParseBody(f);

  if (isBody) RTF_PutStr("</body>\n");

  RTF_PutStr("</html>\n");

  return 0;
}

/**************************************/

void Initialize()
{
  int i;

  for (i=0;i<MAX_LEVELS;i++)
      style_state[i].s=s_plain;

  curr_style.s=s_plain;
  curr_heading = s_plain;

  // Set default styles maping
  style_mappings[0][0]=0;
  for(i=1;i<=6;i++)
      sprintf(style_mappings[i],"%d",256-i);
}

/**************************************/

int RTF_FindCharset(FILE * f)
{
  char ch;
  char code[CHARSET_MAX_LENGTH];
  int metadata = 0;
  int i = 0;

  while ( !feof(f) )
  {
    ch = RTF_GetChar( f );
    if ( ch == '\\' ) 
    {
      metadata++;
    }
    if ( metadata == 2 ) // the second metadata is the charset used
    {
      if ( ch != '\\' ) 
      {
        code[i] = ch;
        i++;
      }
    }
    if ( metadata > 2 )
    {
      code[i] = '\0';
      break;
    }
  }


  for ( i = 0; i < CHARSET_NUMBER ; i++)
  {
    if ( strcmp( (const char *)charset[i], (const char *) code ) == 0 )
    {
      charset_table = charset_variablename[i];
      break;  
    };
  }
  if ( i == CHARSET_NUMBER )
  {
    charset_table = charset_variablename[CHARSET_DEFAULT];
  }

  return 1; // always true!
}

/**************************************/

int main(int argc,char** argv)
{
  int rv = 0;
  FILE *f = NULL;

  Initialize();
  
  if ( argc > 1)
  {
    if( strcmp(argv[1],"--help")==0 || strcmp(argv[1],"-H")==0 )
    {
      printf("Use: %s [rtf_filename]\n",argv[0]);
      rv = 0;
    } else if ( strcmp(argv[1],"--version")==0 || strcmp(argv[1],"-V")==0 ) {
      printf("rtf2html version 1.2\n");
      rv = 0;
    }
    else
    {
      rv = openfile(argv[1], &f);
      if ( rv ) rv = RTF_FindCharset(f);
      if ( rv ) 
      {
        rewind(f);
        rv = RTF_Parse(f);
      }
      if ( rv ) rv = closefile(f);
    }
  }
  else
  {
    printf("Use: %s [rtf_filename]\n",argv[0]);
  }
  return rv;
}
