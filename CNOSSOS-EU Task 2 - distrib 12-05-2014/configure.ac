AC_INIT([cnossos], [1.0], [carl@casalom.se])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CXX
AC_ARG_WITH([octave],AS_HELP_STRING([--without-octave],[skip gnu octave plugin]))
AS_IF([test "x$with_octave" != "xno"],
      [CHECK_FOR_OCTAVE([have_octave=yes], [have_octave=no])],
      [have_octave=no])
AS_IF([test "x$have_octave" = "xyes"],
      [yes_we_have_octave],
      [AS_IF([test "x$with_octave" = "xyes"],
             [AC_MSG_ERROR([required gnu octave but not found])]
      )]
)
#AC_SUBST(CPPFLAGS, "$CPPFLAGS -I<path>")
AC_CHECK_HEADERS([octave/oct.h oct.h],,AC_MSG_ERROR([requires octave header files]))
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile octave/Makefile])
AC_OUTPUT

#https://github.com/NFFT/nfft/commit/1240acfc87606dec9b02081553826ec041db72b6