//
// EmailNotification.h
//
// A simple record of a notification for a webpage.
// It contains a tuple of four strings.
//
// $Id: EmailNotification.h,v 1.1.2.1 2001/07/26 03:12:42 grdetil Exp $
//
// $Log: EmailNotification.h,v $
// Revision 1.1.2.1  2001/07/26 03:12:42  grdetil
// * htcommon/defaults.cc: Add new attributes htnotify_replyto,
//   htnotify_webmaster, htnotify_prefix_file, htnotify_suffix_file.
// * htdoc/attrs.html, cf_by{name,prog}.html: Document them.
// * htnotify/htnotify.cc, htnotify/EmailNotification.{h,cc},
//   htnotify/Makefile.in: Added in code from Richard Beton
//   <richard.beton@roke.co.uk> to collect multiple URLs per e-mail
//   address and allow customization of notification messages by reading
//   in header/footer text as designated by the new attributes above.
//
// Revision
// Initial CVS
//
//
#ifndef _EmailNotification_h_
#define _EmailNotification_h_
#include "Object.h"
#include "htString.h"

class EmailNotification : public Object
{
public:
    EmailNotification (char* date, char* email, char* url, char* subject);

    //
    //accessors
    //
    String getDate()    const { return date; }
    String getEmail()   const { return email; }
    String getUrl()     const { return url; }
    String getSubject() const { return subject; }

private:
    String date;
    String email;
    String url;
    String subject;
};

#endif
