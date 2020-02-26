AC_INIT([cnossos], [1.0], [carl@casalom.se])
AM_INIT_AUTOMAKE([foreign -Wall -Wno-extra-portability -Werror subdir-objects])

# global deps
AC_PROG_CXX

# libtool for shared libraries
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

AC_CONFIG_LIBOBJ_DIR([lib])

# AC_ARG_WITH([octave],AS_HELP_STRING([--without-octave],[skip gnu octave plugin]))
# AS_IF([test "x$with_octave" != "xno"],
#       [CHECK_FOR_OCTAVE([have_octave=yes], [have_octave=no])],
#       [have_octave=no])
# AS_IF([test "x$have_octave" = "xyes"],
#       [yes_we_have_octave],
#       [AS_IF([test "x$with_octave" = "xyes"],
#              [AC_MSG_ERROR([required gnu octave but not found])]
#       )]
# )
#AC_SUBST(CPPFLAGS, "$CPPFLAGS -I<path>")
# AC_CHECK_HEADERS([octave/oct.h oct.h],,AC_MSG_ERROR([requires octave header files]))

AC_ARG_WITH(octave,
      [AC_HELP_STRING([--with-octave=DIR],
      [the directory where Octave header files are installed])],
      octave_dir=${withval},octave_dir="no")

AC_MSG_CHECKING([whether to check for GNU Octave])

AM_CONDITIONAL([WANT_OCTAVE], [test x$octave_dir != xno])

AM_CONDITIONAL([GCC], [test "$CC" = "gcc"])

if test "x${octave_dir}" = "xno"; then
      AC_MSG_RESULT([no])
else
      AC_MSG_RESULT([yes])
      # Octave root
      AX_CHECK_DIR([${octave_dir}],[],
      [AC_MSG_ERROR([Please supply a valid path to a GNU Octave headers directory or run configure without the option --with-octave.])])
      OCTAVE_CPPFLAGS="-I${octave_dir} -fPIC"
      AC_SUBST(OCTAVE_CPPFLAGS)
      # octave_mexext=".mex"
      AC_PATH_PROG([MKOCTPATH],[mkoctfile])
fi


AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

#https://github.com/NFFT/nfft/commit/1240acfc87606dec9b02081553826ec041db72b6
