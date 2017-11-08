//
// HtZlibCodec.cc
//
// HtZlibCodec: Provide a generic access to the zlib compression routines.
//              If zlib is not present, encode and decode are simply 
//              assignment functions.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtZlibCodec.cc,v 1.4 2004/05/28 13:15:12 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtZlibCodec.h"
#include "defaults.h" // For "config"

#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
#include <zlib.h>
#endif


HtZlibCodec::HtZlibCodec()
{
}

HtZlibCodec::~HtZlibCodec()
{
}

String HtZlibCodec::encode(const String &str) const
{
  String s = str;
#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
  HtConfiguration* config= HtConfiguration::config();
  static int cf=config->Value("compression_level",0);    
  if (cf) {
    //
    // Now compress s into c_s
    //
    unsigned char c_buffer[16384];
    String c_s;
    z_stream c_stream; /* compression stream */
    c_stream.zalloc=(alloc_func)0;
    c_stream.zfree=(free_func)0;
    c_stream.opaque=(voidpf)0;
    // Get compression factor, default to best
    if (cf<-1) cf=-1; else if (cf>9) cf=9;
    int err=deflateInit(&c_stream,cf);
    if (err!=Z_OK) return 0;
    int len=s.length();
    c_stream.next_in=(Bytef*)(char *)s;
    c_stream.avail_in=len;
    while (err==Z_OK && c_stream.total_in!=(uLong)len) {
      c_stream.next_out=c_buffer;
      c_stream.avail_out=sizeof(c_buffer);
      err=deflate(&c_stream,Z_NO_FLUSH);
      c_s.append((char *)c_buffer,c_stream.next_out-c_buffer);
    }
    // Finish the stream
    for (;;) {
      c_stream.next_out=c_buffer;
      c_stream.avail_out=sizeof(c_buffer);
      err=deflate(&c_stream,Z_FINISH);
      c_s.append((char *)c_buffer,c_stream.next_out-c_buffer);
      if (err==Z_STREAM_END) break;
      //CHECK_ERR(err, "deflate");
    }
    err=deflateEnd(&c_stream); 
    s=c_s;
  }
#endif // HAVE_LIBZ && HAVE_ZLIB_H
  return s;
}


String HtZlibCodec::decode(const String &str) const
{
  String s = str;
#if defined(HAVE_LIBZ) && defined(HAVE_ZLIB_H)
  HtConfiguration* config= HtConfiguration::config();
  static int cf=config->Value("compression_level",0);    
  if (cf) {
    String c_s;
    // Decompress stream
    unsigned char c_buffer[16384];
    z_stream d_stream;
    d_stream.zalloc=(alloc_func)0;
    d_stream.zfree=(free_func)0;
    d_stream.opaque=(voidpf)0;
    
    unsigned int len=s.length();
    d_stream.next_in=(Bytef*)(char *)s;
    d_stream.avail_in=len;
    
    int err=inflateInit(&d_stream);
    if (err!=Z_OK) return 1;
    
    while (err==Z_OK && d_stream.total_in<len) {
      d_stream.next_out=c_buffer;
      d_stream.avail_out=sizeof(c_buffer);
      err=inflate(&d_stream,Z_NO_FLUSH);
      c_s.append((char *)c_buffer,d_stream.next_out-c_buffer);
      if (err==Z_STREAM_END) break;
    }
    
    err=inflateEnd(&d_stream);
    s=c_s;
  }
#endif // HAVE_LIBZ && HAVE_ZLIB_H
  return s;
}


// Canonical singleton interface.
HtZlibCodec *
HtZlibCodec::instance()
{
  static HtZlibCodec *_instance = 0;

  if (_instance == 0)
  {
    _instance = new HtZlibCodec();
  }

  return _instance;
}


// End of HtZlibCodec.cc
