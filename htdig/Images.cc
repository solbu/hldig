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
// $Id: Images.cc,v 1.5.2.1 2000/02/19 05:02:45 ghutchis Exp $
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
		if (c.Open() == NOTOK)
			return 0;
		if (c.Assign_Port(Url.port()) == NOTOK)
			return 0;
		if (c.Assign_Server(Url.host()) == NOTOK)
			return 0;

		if (c.Connect(1) == NOTOK)
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

		c.Write(command);

		String	line;
		while (1)
		{
			c.Read_Line(line, "\n");
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
		c.Close();
		images.Add(u, is);
		return is->Size;
	}
}


