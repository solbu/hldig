//
// Images.cc
//
// Images: Issue an HTTP request to retrieve the size of an image from
//         the content-length field.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Images.cc,v 1.5 1999/09/11 05:03:50 ghutchis Exp $
//

#include "Images.h"
#include "htdig.h"
#include "URL.h"
#include "Connection.h"


class ImageSize : public Object
{
public:
	ImageSize() {}
	~ImageSize();

	int	Size;
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


