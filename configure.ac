#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(vitas.c, 0.1, yiguopan@126.com)
AC_CONFIG_SRCDIR([vitas.c])
#AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE(vitas.c, 0.1)
# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-levent':
AC_CHECK_LIB([event], [leveldb])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 gethostbyname inet_ntoa memset socket strchr strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
