//
// Connection.h
//
// Connection: This class forms a easy to use interface to the berkeley
//             tcp socket library. All the calls are basically the same, 
//             but the parameters do not have any stray _addr or _in
//             mixed in...
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Connection.h,v 1.3.2.4 2000/03/02 17:58:48 angus Exp $
//

#ifndef _Connection_h_
#define	_Connection_h_

#include "Object.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

class String;

class Connection : public Object
{
public:
    // Constructors & Destructors
    Connection();
    Connection(int socket);
    ~Connection();

    // (De)initialization
    int				Open(int priv = 0);
    int				Close();
    int				Ndelay();
    int				Nondelay();
    int                         Timeout(int value);
    int				Retries(int value);
    int  			WaitTime(unsigned int _wt);

    // Port stuff
    int				Assign_Port(int port = 0);
    int				Assign_Port(char *service);
    int				Get_Port();
    inline int			Is_Privileged();

    // Host stuff
    int				Assign_Server(const String& name);
    int				Assign_Server(unsigned int addr = INADDR_ANY);
    char		       *Get_Server()		{return server_name;}

    // Connection establishment
    int				Connect();
    Connection		       *Accept(int priv = 0);
    Connection		       *Accept_Privileged();

    // Registration things
    int				Bind();
    int				Listen(int n = 5);

    // IO
    String		       *Read_Line(String &, char *terminator = "\n");
    char		       *Read_Line(char *buffer, int maxlength, char *terminator = "\n");
    String		       *Read_Line(char *terminator = "\n");
    virtual int			Read_Char();
    int				Write_Line(char *buffer, char *eol = "\n");
    
    int				Write(char *buffer, int maxlength = -1);
    int				Read(char *buffer, int maxlength);
    
    int				Read_Partial(char *buffer, int maxlength);
    int				Write_Partial(char *buffer, int maxlength);
    void			Stop_IO()		{need_io_stop = 1;}

    // Access to socket number
    char		       *Socket_as_String();
    int				Get_Socket()		{return sock;}
    int				IsOpen()		{return sock >= 0;}
    int				IsConnected()		{return connected;}

    // Access to info about remote socket
    char		       *Get_PeerIP();
    char		       *Get_Peername();

    // A method to re-initialize the buffer
    virtual void        	Flush();

private:
    //
    // For buffered IO we will need a buffer
    //
    enum			{BUFFER_SIZE = 8192};
    char			buffer[BUFFER_SIZE];
    int				pos, pos_max;

    int				sock;
    struct sockaddr_in		server;
    int				connected;
    char			*peer;
    char			*server_name;
    int				need_io_stop;
    int                         timeout_value;
    int                         retry_value;

    unsigned int                wait_time;   // time to wait after an
                                             // unsuccessful connection
};


//*************************************************************************
// inline int Connection::Is_Privileged()
// PURPOSE:
//   Return whether the port is priveleged or not.
//
inline int Connection::Is_Privileged()
{
    return server.sin_port < 1023;
}


//
// Get arround the lack of gethostip() library call...  There is a gethostname()
// call but we want the IP address, not the name!
// The call will put the ASCII string representing the IP address in the supplied
// buffer and it will also return the 4 byte unsigned long equivalent of it.
// The ip buffer can be null...
//
unsigned int gethostip(char *ip = 0, int length = 0);

#endif
