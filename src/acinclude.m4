dnl -- Begin zlib autoconf macro
dnl Copyright (c) 2002 Patrick McFarland
dnl Under the GPL License
dnl When copying, include from Begin to End zlib autoconf macro, including
dnl those tags, so others can easily copy it too. (Maybe someday this will
dnl become zlib.m4?)
dnl
dnl AM_PATH_ZLIB([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Tests for zlib, outputs ZLIB_VERSION, ZLIB_LIBS, and ZLIB_CFLAGS
AC_DEFUN(AM_PATH_ZLIB, 
[dnl
dnl
dnl
AC_ARG_WITH(zlib-prefix,[  --with-zlib-prefix=PFX  Prefix where zlib is installed (optional)], zlib_prefix="$withval", zlib_prefix="")
min_zlib_version=ifelse([$1], ,1.1.0,$1)
AC_MSG_CHECKING(for zlib - version >= $min_zlib_version)

dnl Deal with X11R6 stupidity first. Some distros include zlib with X11R6,
dnl and we cant use that one. We have to bomb if we find it.

if test [-e /usr/X11R6/lib/libz.a] ; then 
AC_MSG_RESULT(error)
echo
echo configure: Found a copy of zlib from X11R6
echo configure: Please delete /usr/X11R6/lib/libz.a and /usr/X11R6/include/unzip.h
echo "           so we can use the real zlib on your system."
AC_MSG_ERROR(Remove these files and rerun this script)
fi
if test [-e /usr/X11R6/include/unzip.h] ; then 
AC_MSG_RESULT(error)
echo
echo configure: Found a copy of zlib from X11R6
echo configure: Please delete /usr/X11R6/lib/libz.a and /usr/X11R6/include/unzip.h
echo "           so we can use the real zlib on your system."
AC_MSG_ERROR(Remove these files and rerun this script)
fi

dnl Okay, Stupidy Check is done.

tempLIBS="$LIBS"
tempCFLAGS="$CFLAGS"
if test x$zlib_prefix != x ; then
ZLIB_LIBS="-L$zlib_prefix"
ZLIB_CFLAGS="-I$zlib_prefix"
fi
ZLIB_LIBS="$ZLIB_LIBS -lz"
LIBS="$LIBS $ZLIB_LIBS"
CFLAGS="$CFLAGS $ZLIB_CFLAGS"
with_zlib=no
AC_TRY_RUN([
#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
int major, minor, micro, zlib_major_version, zlib_minor_version, zlib_micro_version;

char *zlibver;
char *tmp_version;

zlibver = ZLIB_VERSION;

	{ FILE *fp = fopen("conf.zlibtest", "a"); 
		if ( fp ) { 
		fprintf(fp, "%s", zlibver);
		fclose(fp); 
		}
	}

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_zlib_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string for min_zlib_version.\n", "$min_zlib_version");
     exit(1);
   }
   if (sscanf(zlibver, "%d.%d.%d", &zlib_major_version, &zlib_minor_version, &zlib_micro_version) != 3) {
printf("%s, bad version string given by zlib, sometimes due to very old zlibs \n didnt correctly define their version. Please upgrade if you are running \n an old zlib.\n", "zlibver");
	exit(1);
	}

   if ((zlib_major_version > major) ||
      ((zlib_major_version == major) && (zlib_minor_version > minor)) ||
      ((zlib_major_version == major) && (zlib_minor_version == minor) && (zlib_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      return 1;
    }
}
],with_zlib=yes,,[AC_MSG_WARN(Cross Compiling, Assuming zlib is avalible)])

if test x$with_zlib = xyes; then
	AC_MSG_RESULT(yes)
	ZLIB_VERSION=$(<conf.zlibtest)
	ifelse([$2], , :, [$2])
else
	AC_MSG_RESULT(no)
	ZLIB_CFLAGS=""
	ZLIB_LIBS=""
	ZLIB_VERSION=""
	ifelse([$3], , :, [$3])
fi
LIBS="$tempLIBS"
CFLAGS="$tempCFLAGS"
rm conf.zlibtest
AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_VERSION)
AC_SUBST(ZLIB_LIBS)
])
dnl -- End zlib autoconf macro

dnl ----

dnl -- Begin libpng autoconf macro
dnl Copyright (c) 2002 Patrick McFarland
dnl Under the GPL License
dnl When copying, include from Begin to End libpng autoconf macro, including
dnl those tags, so others can easily copy it too. (Maybe someday this will
dnl become libpng.m4?)
dnl
dnl AM_PATH_LIBPNG([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Tests for libpng, outputs LIBPNG_VERSION, LIBPNG_LIBS, and LIBPNG_CFLAGS
AC_DEFUN(AM_PATH_LIBPNG,
[dnl
dnl
dnl

dnl Comment out this section, and the other labled parts to disable the user
dnl having a choice about being able to disable libpng or not. Recommended
dnl you use AC_MSG_ERROR(LIBPNG >= 1.0.0 is required) for the 
dnl ACTION-IF-NOT-FOUND if you plan on disabling user choice.

dnl <--- disable for no user choice part #1
AC_ARG_ENABLE(libpng,[  --disable-libpng        Build without libpng support ],,enable_libpng=yes)
dnl --->

AC_ARG_WITH(libpng-prefix,[  --with-libpng-prefix=PFX Prefix where libpng is installed (optional)], libpng_prefix="$withval", libpng_prefix="")

min_libpng_version=ifelse([$1], ,1.0.0,$1)
tempLIBS="$LIBS"
tempCFLAGS="$CFLAGS"
if test x$libpng_prefix != x ; then
LIBPNG_LIBS="-L$libpng_prefix"
LIBPNG_CFLAGS="-I$libpng_prefix"
fi
LIBPNG_LIBS="$LIBPNG_LIBS -lpng"
LIBS="$LIBS $LIBPNG_LIBS"
CFLAGS="$CFLAGS $LIBPNG_CFLAGS"

AC_MSG_CHECKING(for libpng - version >= $min_libpng_version)
with_libpng=no

dnl <--- disable for no user choice part #2
if test x$enable_libpng != xno; then
dnl --->

AC_TRY_RUN([
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
int major, minor, micro, libpng_major_version, libpng_minor_version, libpng_micro_version;

char *libpngver;
char *tmp_version;

libpngver = PNG_LIBPNG_VER_STRING;

	{ FILE *fp = fopen("conf.libpngtest", "a"); 
		if ( fp ) { 
		fprintf(fp, "%s", libpngver);
		fclose(fp); 
		}
	}

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_libpng_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string for min_libpng_version.\n", "$min_libpng_version");
     exit(1);
   }
   if (sscanf(libpngver, "%d.%d.%d", &libpng_major_version, &libpng_minor_version, &libpng_micro_version) != 3) {
printf("%s, bad version string given by libpng, sometimes due to very old libpngs \n didnt correctly define their version. Please upgrade if you are running \n an old libpng.\n", "libpngver");
	exit(1);
	}

   if ((libpng_major_version > major) ||
      ((libpng_major_version == major) && (libpng_minor_version > minor)) ||
      ((libpng_major_version == major) && (libpng_minor_version == minor) && (libpng_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      return 1;
    }
}
],with_libpng=yes,,[AC_MSG_WARN(Cross Compiling, Assuming libpng is avalible)])

if test x$with_libpng = xyes; then
	AC_MSG_RESULT(yes)
	LIBPNG_VERSION=$(<conf.libpngtest)
	ifelse([$2], , :, [$2])
else
	AC_MSG_RESULT(no)
	LIBPNG_CFLAGS=""
	LIBPNG_LIBS=""
	LIBPNG_VERSION=""
	ifelse([$3], , :, [$3])
fi
LIBS="$tempLIBS"
CFLAGS="$tempCFLAGS"
rm conf.libpngtest
AC_SUBST(LIBPNG_CFLAGS)
AC_SUBST(LIBPNG_VERSION)
AC_SUBST(LIBPNG_LIBS)

dnl <--- disable for no user choice part #3
else
        AC_MSG_RESULT(disabled by user)
fi
dnl ---> 
])
dnl -- End libpng autoconf macro
