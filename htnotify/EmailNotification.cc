//
// Implementation of EmailNotification tuple class
//
// $Id: EmailNotification.cc,v 1.1.2.1 2001/07/26 03:12:42 grdetil Exp $
//
#if RELEASE
static char     RCSid[] = "$Id: EmailNotification.cc,v 1.1.2.1 2001/07/26 03:12:42 grdetil Exp $";
#endif


#include "EmailNotification.h"


EmailNotification::EmailNotification (char* pDate, char* pEmail,
                                      char* pUrl,  char* pSubject)
{
    date    = pDate;
    email   = pEmail;
    url     = pUrl;
    if (!pSubject || !*pSubject)
    {
        subject = "page expired";
    }
    else
    {
        subject = pSubject;
    }
}

