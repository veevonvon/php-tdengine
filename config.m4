dnl $Id$
dnl config.m4 for extension tdengine

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(tdengine, for tdengine support,
dnl Make sure that the comment is aligned:
dnl [  --with-tdengine             Include tdengine support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(tdengine, whether to enable tdengine support,
dnl Make sure that the comment is aligned:
[  --enable-tdengine           Enable tdengine support])

if test "$PHP_TDENGINE" != "no"; then

  AC_PATH_PROG(PKG_CONFIG, taos, no)
  AC_MSG_CHECKING(for taos)
  if test -x "$PKG_CONFIG" && $PKG_CONFIG --version ; then
    LIBFOO_VERSON=`$PKG_CONFIG --version`
    AC_MSG_RESULT(from taoslib: version $LIBFOO_VERSON)
  else
    AC_MSG_ERROR(taoslib not found)
  fi
  PHP_EVAL_LIBLINE($LIBFOO_LIBDIR, TDENGINE_SHARED_LIBADD)
  PHP_EVAL_INCLINE($LIBFOO_CFLAGS)

  # --with-tdengine -> check with-path
  SEARCH_PATH="/usr/local/taos /usr/local"     # you might want to change this
  SEARCH_FOR="/include/taos.h"  # you most likely want to change this
  if test -r $PHP_TDENGINE/$SEARCH_FOR; then # path given as parameter
    TDENGINE_DIR=$PHP_TDENGINE
  else # search default path list
    AC_MSG_CHECKING([for taoslib files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        TDENGINE_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$TDENGINE_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the tdengine distribution])
  fi

  # --with-tdengine -> add include path
  

  # --with-tdengine -> check for lib and symbol presence
  LIBNAME=taos # you may want to change this
  LIBSYMBOL=taos_init # you most likely want to change this 
  PHP_LIBDIR="driver"
  PHP_INCLUDEDIR="include"
  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    AC_CHECK_LIB($LIBNAME, taos_init,AC_DEFINE(HAVE_TAOS_INIT,1,[TDengine 2.0.5.1 or later]))
    AC_CHECK_LIB($LIBNAME, taos_cleanup,AC_DEFINE(HAVE_TAOS_CLEANUP,1,[TDengine 2.0.5.1 or later]))
    AC_CHECK_LIB($LIBNAME, taos_options,AC_DEFINE(HAVE_TAOS_OPTIONS,1,[TDengine 2.0.5.1 or later]))
    AC_CHECK_LIB($LIBNAME, taos_connect,AC_DEFINE(HAVE_TAOS_CONNECT,1,[TDengine 2.0.5.1 or later]))
    AC_DEFINE(HAVE_TDENGINELIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong tdengine lib version or lib not found, lib path $TDENGINE_DIR/$PHP_LIBDIR])
  ],[
    -L$TDENGINE_DIR/$PHP_LIBDIR -lm
  ])

  PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $TDENGINE_DIR/$PHP_LIBDIR, TDENGINE_SHARED_LIBADD)
  PHP_SUBST(TDENGINE_SHARED_LIBADD)

  PHP_ADD_INCLUDE($TDENGINE_DIR/$PHP_INCLUDEDIR)

  PHP_NEW_EXTENSION(tdengine, tdengine.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
