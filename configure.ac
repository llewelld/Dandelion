#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([Dandelion], [1.01], [david@flypig.co.uk])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/dandelion.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
PKG_CHECK_MODULES([DANDELION], [gtk+-2.0 libglade-2.0 gdk-2.0 gtkglext-1.0 glee glu libxml-2.0 gnet-2.0])
AC_CHECK_LIB([glut], [glutInit], [], [AC_MSG_ERROR([The glut library is missing.])])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h sys/timeb.h GLee.h])

# Checks for compiler characteristics
AC_C_BIGENDIAN

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_SEARCH_LIBS([GLeeEnabled], [glee GLee], [], [])
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor ftime pow sqrt])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 assets/Makefile
                 assets/icons/Makefile
                 dandeclientsrc/Makefile])
AC_OUTPUT
