#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sst, 2.0, stsp)
AM_INIT_AUTOMAKE([])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.14.5)
AM_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

AC_CHECK_TOOL(have_xmlto, xmlto, "no")
if test "$have_xmlto" = "no";
then
    AC_MSG_ERROR([xmlto is not installed])
fi
AC_CHECK_TOOL(have_python, python, "no")
if test "$have_python" = "no";
then
    AC_MSG_ERROR([python is not installed])
fi

# Checks for libraries.
AC_CHECK_LIB(m, floor, [],
    AC_MSG_ERROR([math library is missing on your system.]))
AC_CHECK_LIB(ncurses, newwin, [],
    AC_MSG_ERROR([ncurses library is missing on your system.]))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([curses.h],
    [], AC_MSG_ERROR([curses headers are missing on your system.]))
AC_CHECK_HEADERS([fcntl.h sys/ioctl.h linux/kd.h])
# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([pow sqrt floor strcasecmp strncasecmp strchr strstr atexit], [],
    AC_MSG_ERROR([some functions are missing on your system.]))

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    po/Makefile.in
    doc/Makefile
])
AC_OUTPUT
