dnl Process this file with autoconf to produce a configure script.

AC_INIT(examples, 1.0, chikoos@example.com)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src)

AM_INIT_AUTOMAKE([1.0 no-define foreign])

AC_PROG_CC
AC_PROG_CXX

AC_C_CONST
AC_TYPE_SIZE_T

AC_CHECK_HEADERS(GL/glut.h GL/gl.h)

AC_OUTPUT(
          Makefile
          src/Makefile
          )

