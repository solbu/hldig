//
// Images.cc
//
// Implementation of Images
//
// $Log: Images.cc,v $
// Revision 1.3  1998/10/12 02:04:00  ghutchis
//
// Updated Makefiles and configure variables.
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Images.cc,v 1.3 1998/10/12 02:04:00 ghutchis Exp $";
#endif

#include "Images.h"
#include "htdig.h"
#include <URL.h>
#include <Connection.h>


class ImageSize : public Object
{
public:
					ImageSize()				{}
					~ImageSize();

	int				Size;
};


ImageSize::~ImageSize()
{
}


//*******************************************************************************
// Images::Images()
//
Images::Images()
{
}


//*******************************************************************************
// Images::~Images()
//
Images::~Images()
{
}


//*******************************************************************************
// int Images::Sizeof(char *url)
//
int
Images::Sizeof(char *url)
{
	String	u = url;
	URL		Url(url);
	if (strcmp(Url.service(), "http") != 0)
		return 0;

	u.lowercase();
	ImageSize	*is = (ImageSize *) images[u];
	if (is)
	{
		return is->Size;
	}
	else
	{
		is = new ImageSize;
		is->Size = 0;
		Connection	c;
		if (c.open() == NOTOK)
			return 0;
		if (c.assign_port(Url.port()) == NOTOK)
			return 0;
		if (c.assign_server(Url.host()) == NOTOK)
			return 0;

		if (c.connect(1) == NOTOK)
		{
			is->Size = 0;
			images.Add(u, is);
			return 0;
		}

		String		command = "HEAD ";
		command << Url.path() << " HTTP/1.0\r\n";
		command << "User-Agent: " << config["user_agent"] << "/";
		command << VERSION << " (" << config["maintainer"] << ")\r\n";
		command << "\r\n";

		c.write(command);

		String	line;
		while (1)
		{
			c.read_line(line, "\n");
			line.chop('\r');
			if (line.length() == 0)
				break;
			else
			{
				char	*name = strtok(line, " ");
				if (mystrcasecmp(name, "content-length:") == 0)
				{
					is->Size = atoi(strtok(0, " "));
					break;
				}
			}
		}
		c.close();
		images.Add(u, is);
		return is->Size;
	}
}


