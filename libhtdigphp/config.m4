dnl $Id: config.m4,v 1.1 2004/03/20 01:31:21 nealr Exp $
dnl config.m4 for extension HtDig

PHP_ARG_WITH(bz2, for HtDig support,
[  --with-bz2[=DIR]        Include HtDig support])

if test "$PHP_HTDIG" != "no"; then
  if test -r $PHP_HTDIG/include/libhtdig_api.h; then
    HTDIG_DIR=$PHP_HTDIG
  else
    AC_MSG_CHECKING(for HtDig in default path)
    for i in /usr/local /usr; do
      if test -r $i/include/libhtdig_api.h; then
        HTDIG_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$HTDIG_DIR"; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR(Please reinstall the HtDig distribution)
  fi

  PHP_ADD_INCLUDE($HTDIG_DIR/include)

  PHP_SUBST(HTDIG_SHARED_LIBADD)
  PHP_ADD_LIBRARY_WITH_PATH(bz2, $HTDIG_DIR/lib, HTDIG_SHARED_LIBADD)
  #AC_CHECK_LIB(bz2, BZ2_bzerror, [AC_DEFINE(HAVE_BZ2,1,[ ])], [AC_MSG_ERROR(bz2 module requires libbz2 >= 1.0.0)],)

  PHP_EXTENSION(bz2, $ext_shared)
fi
