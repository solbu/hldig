//
// SDSU.cc
//
// Implementation of SDSU
//
// $Log: SDSU.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: SDSU.cc,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $";
#endif

#include <String.h>
#include <time.h>
#include "SDSU.h"

static char *months[12] =
{
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
};


char *SDSU_bar()
{
	return "<IMG SRC=\"http://www.sdsu.edu/graphics/divider_red.gif\" width=100% height=6\nalt=\"----------------------------------------------------------------------\">";
}


char *SDSU_narrow_bar()
{
	return "<IMG SRC=\"http://www.sdsu.edu/graphics/narrow_divider_red.gif\" width=100% height=4\nalt=\"       --------------------------------------------------------\">";
}


char *SDSU_BOTTOM(char *left, char *right)
{
	static String	s;
	s = "<center>";
	if (left)
		s << "<A HREF=\"" << left << "\"><IMG SRC=\"http://www.sdsu.edu/graphics/BACK.gif\" border=0 alt=BACK></A>\n";
	else
		s << "<IMG SRC=\"http://www.sdsu.edu/graphics/BLANK.gif\" border=0 alt=BACK>\n";
	s << "<A HREF=\"http://www.sdsu.edu/index.html\"><IMG SRC=\"http://www.sdsu.edu/graphics/HOME.gif\" border=0 alt=HOME></A>\n";
	if (right)
		s << "<A HREF=\"" << right << "\"><IMG SRC=\"http://www.sdsu.edu/graphics/NEXT.gif\" border=0 alt=NEXT></A>\n";
	else
		s << "<IMG SRC=\"http://www.sdsu.edu/graphics/BLANK.gif\" border=0 alt=NEXT>\n";
	s << "</center>\n";

	return s;
}


char *SDSU_logo(char *title)
{
	static String	s;
	s = "<h2><img align=bottom src=\"http://www.sdsu.edu/graphics/sdsu_logo_6.gif\" ";
	s << "alt=SDSU>\n";
	s << title << "</h2>\n";
	s << "<center>San Diego State University<i> -- This page generated ";

	time_t	now = time(0);
	struct tm	*tm = localtime(&now);

	s << months[tm->tm_mon] << ' ' << tm->tm_mday << ", " << tm->tm_year + 1900;
	s << "</i></center>\n";
	s << SDSU_bar();

	return s;
}


