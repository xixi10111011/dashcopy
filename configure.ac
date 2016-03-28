#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([dashcopy], [0.1], [dashcopy@github])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

# Remember externally set CFLAGS
EXTERNAL_CFLAGS="$CFLAGS"

# Checks for programs.
AC_PROG_CC_C99
AC_PROG_CPP

# Reset the externally set CFLAGS after calling AC_PROG*
CFLAGS="$EXTERNAL_CFLAGS"

AC_LANG([C])

# Checks for libraries.
PKG_CHECK_MODULES([libcurl], [curl])
PKG_CHECK_MODULES([libxml2], [libxml-2.0])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdio.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.

# Add debug support
AC_ARG_ENABLE(debug,
  AS_HELP_STRING(
     [--enable-debug],
     [enable debugging, default: no]),
     [case "${enableval}" in
       yes) debug=true ;;
       no)  debug=false ;;
       *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
      esac],
      [debug=false])
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")
AM_COND_IF(DEBUG,
      AC_DEFINE(DEBUG, 1, [Define to 0 if this is a release build]),
      AC_DEFINE(DEBUG, 0, [Define to 1 or higher if this is a debug build]))

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
