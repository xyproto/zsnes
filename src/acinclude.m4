dnl -- Begin zlib autoconf macro
dnl Copyright (c) 2002 Patrick McFarland
dnl Under the GPL License
dnl When copying, include from Begin to End zlib autoconf macro, including
dnl those tags, so others can easily copy it too. (Maybe someday this will
dnl become zlib.m4?)
dnl
dnl AM_PATH_ZLIB([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Tests for zlib, outputs ZLIB_VERSION, ZLIB_LIBS, and ZLIB_CFLAGS
AC_DEFUN([AM_PATH_ZLIB],
[dnl
dnl
dnl
AC_ARG_WITH(zlib-prefix,[  --with-zlib-prefix=PFX  Prefix where zlib is installed (optional)], zlib_prefix="$withval", zlib_prefix="")
min_zlib_version=ifelse([$1], ,1.1.0,$1)
AC_MSG_CHECKING(for zlib - version >= $min_zlib_version)

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

char* my_strdup (char *str)
{
  char *new_str;

  if (str)
  {
    new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
    strcpy (new_str, str);
  }
  else new_str = NULL;

  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro, zlib_major_version, zlib_minor_version, zlib_micro_version;

  char *zlibver, *tmp_version;

  zlibver = ZLIB_VERSION;

  FILE *fp = fopen("conf.zlibtest", "a");
  if ( fp ) {
    fprintf(fp, "%s", zlibver);
    fclose(fp);
  }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_zlib_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
    printf("%s, bad version string for\n\tmin_zlib_version... ", "$min_zlib_version");
    exit(1);
  }
  if (sscanf(zlibver, "%d.%d.%d", &zlib_major_version, &zlib_minor_version, &zlib_micro_version) != 3) {
    printf("%s, bad version string given\n", zlibver);
    puts("\tby zlib, sometimes due to very old zlibs that didnt correctly");
    printf("\tdefine their version. Please upgrade if you are running an\n\told zlib... ");
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
AC_DEFUN([AM_PATH_LIBPNG],
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

min_libpng_version=ifelse([$1], ,1.2.0,$1)
tempLIBS="$LIBS"
tempCFLAGS="$CFLAGS"
if test x$libpng_prefix != x ; then
  LIBPNG_LIBS="-L$libpng_prefix"
  LIBPNG_CFLAGS="-I$libpng_prefix"
fi
LIBPNG_LIBS="$LIBPNG_LIBS -lpng -lm"
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

  char* my_strdup (char *str)
  {
    char *new_str;

    if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
    else new_str = NULL;

    return new_str;
  }

  int main (int argc, char *argv[])
  {
    int major, minor, micro, libpng_major_version, libpng_minor_version, libpng_micro_version;

    char *libpngver, *tmp_version;

    libpngver = PNG_LIBPNG_VER_STRING;

    FILE *fp = fopen("conf.libpngtest", "a");
    if ( fp ) {
      fprintf(fp, "%s", libpngver);
      fclose(fp);
    }

    /* HP/UX 9 (%@#!) writes to sscanf strings */
    tmp_version = my_strdup("$min_libpng_version");
    if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
      printf("%s, bad version string for\n\tmin_libpng_version... ", "$min_libpng_version");
      exit(1);
    }
    if (sscanf(libpngver, "%d.%d.%d", &libpng_major_version, &libpng_minor_version, &libpng_micro_version) != 3) {
      printf("%s, bad version string given\n", libpngver);
      puts("\tby libpng, sometimes due to very old libpngs that didnt correctly");
      printf("\tdefine their version. Please upgrade if you are running an\n\told libpng... ");
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

dnl ----

dnl -- Begin custom cpu detection autoconf macro
dnl Copyright (c) 2005 Nach, grinvader
dnl Under the GPL License
dnl When copying, include from Begin to End custom cpu detection autoconf macro,
dnl including those tags, so others can easily copy it too.
dnl
dnl AM_CPU_DETECT([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Tests and reads the first entry in /proc/cpuinfo, outputs CPU_INFO
AC_DEFUN([AM_CPU_DETECT],
[
CPU_INFO=""

AC_MSG_CHECKING(for cpu info)
AC_ARG_ENABLE(cpucheck, [  --disable-cpucheck      Do not try to autodetect cpu ],,enable_cpucheck=yes)

if test x$enable_cpucheck != xno; then
  AC_TRY_RUN([
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>

  int main()
  {
    char line[256], key[40], arg[216], *cpu = 0;
    const char *pattern = " %39[^:]: %215[ -~]"; // for sscanf

    char model_name[216], flags[216], cpu_family[216], vendor_id[216], model[216];

    FILE *fp;

    *model_name = 0;
    *flags = 0;
    *cpu_family = 0;
    *vendor_id = 0;
    *model = 0;

    if ((fp = fopen("/proc/cpuinfo", "r")))
    {
      while (fgets(line, sizeof(line), fp) && sscanf(line, pattern, key, arg) == 2)
      {
        if (!strncmp(key, "model name", strlen("model name")) && !*model_name)
        { strcpy(model_name, arg); }
        else if (!strncmp(key, "flags", strlen("flags")) && !*flags)
        { strcpy(flags, arg); }
        else if (!strncmp(key, "cpu family", strlen("cpu family")) && !*cpu_family)
        { strcpy(cpu_family, arg); }
        else if (!strncmp(key, "vendor_id", strlen("vendor_id")) && !*vendor_id)
        { strcpy(vendor_id, arg); }
        else if (!strncmp(key, "model", strlen("model")) && !*model)
        { strcpy(model, arg); }
      }
      fclose(fp);

      if (!strcmp(vendor_id, "AuthenticAMD") || strstr(model_name, "AMD"))
      {
        if (strstr(flags, "mmx"))
        {
          #if __GNUC__ > 2
          if (strstr(flags, "3dnow"))
          {
            if (strstr(flags, "3dnowext"))
            {
              #if __GNUC__ > 3 || __GNUC_MINOR__ > 0
              if (strstr(flags, "sse"))
              {
                #if __GNUC__ > 3 || __GNUC_MINOR__ > 3
                if (strstr(flags, "sse2") && strstr(flags, "lm")) //Need two checks to protect Semprons
                {
                  if (strstr(model_name, "Opteron")) { cpu = "opteron"; }
                  else { cpu = (strstr(model_name, "Athlon(tm) 64")) ? cpu = "athlon64" : "k8"; }
                }
                #endif

                if (!cpu)
                {
                  if (strstr(model_name, "Athlon(tm) 4")) { cpu = "athlon-4"; }
                  else { cpu = (strstr(model_name, "Athlon(tm) MP")) ? "athlon-mp" : "athlon-xp"; }
                }
              }

              if (!cpu && (atoi(model) > 3)) { cpu = "athlon-tbird"; }
              #endif

              if (!cpu) { cpu = "athlon"; }
            }

            #if __GNUC__ > 3 || __GNUC_MINOR__ > 0
            if (!cpu)
            {
              int model_num = atoi(model);
              cpu = ((model_num == 9) || (model_num >= 13)) ? "k6-3" : "k6-2";
            }
            #endif
          }
          #endif

          if (!cpu && (atoi(cpu_family) > 5)) { cpu = "k6"; }
        }
      }
      else if (!strcmp(vendor_id, "GenuineIntel") || strstr(model_name, "Intel"))
      {
        #if __GNUC__ > 2
        if (strstr(flags, "mmx"))
        {
          if (strstr(flags, "sse"))
          {
            if (strstr(flags, "sse2"))
            {
              #if __GNUC__ > 3 || __GNUC_MINOR__ > 2
              if (strstr(flags, "pni"))
              {
                cpu = (strstr(flags, "lm")) ? "nocona" : "prescott";
              }
              #endif

              if (!cpu)
              {
                if (strstr(model_name, "Pentium(R) M"))
                {
                  #if __GNUC__ > 3 || __GNUC_MINOR__ > 3
                  cpu = "pentium-m";
                  #else
                  cpu = "pentium3";
                  #endif
                }
                else
                {
                  #if __GNUC__ > 3 || __GNUC_MINOR__ > 2
                  if (strstr(model_name, "Mobile")) { cpu = "pentium4m"; }
                  #endif

                  if (!cpu) { cpu = "pentium4"; }
                }
              }
            }
            else { cpu = "pentium3"; }
          }
          else { cpu = (!strcmp(cpu_family, "6")) ? "pentium2" : "pentium-mmx"; }
        }

        if (!cpu)
        {
          int family = atoi(cpu_family);
          if (family > 5) { cpu = "pentiumpro"; }
          else if (family == 5) { cpu = "pentium"; }
        }
        #endif
      }
      #if __GNUC__ > 2
      else if (strstr(model_name, "VIA"))
      {
        if (strstr(flags, "mmx"))
        {
          #if __GNUC__ > 3 || __GNUC_MINOR__ > 2
          if (strstr(flags, "3dnow")) { cpu = "c3"; }
          #if __GNUC__ > 3 || __GNUC_MINOR__ > 3
          else if (strstr(flags, "sse")) { cpu = "c3-2"; }
          #endif
          #endif
       }
     }
     else if (strstr(model_name, "WinChip"))
     {
        #if __GNUC__ > 3 || __GNUC_MINOR__ > 2
        if (strstr(flags, "mmx"))
        {
          cpu = (strstr(flags, "3dnow")) ? "winchip2" : "winchip-c6";
        }
        #endif
      }
      #endif

      if (!cpu)
      {
        int family = atoi(cpu_family);
        if (family > 5) { cpu = "i686"; }
        else if (family == 5) { cpu = "i586"; }
        else if (family == 4) { cpu = "i486"; }
        else { cpu = "i386"; }
      }

      if ((fp = fopen("conf.cpuchk", "a")))
      {
        fprintf(fp, "%s", (cpu) ? cpu : "");
        fclose(fp);
        return 0;
      }
    }
    return 1;
  }
  ],cpu_found=yes)

  if test x$cpu_found = xyes; then
    AC_MSG_RESULT(found)
    CPU_INFO=$(<conf.cpuchk)
    ifelse([$1], , :, [$1])
    rm conf.cpuchk
  else
    AC_MSG_RESULT(not found)
    ifelse([$2], , :, [$2])
  fi
  AC_SUBST(CPU_INFO)

else
  AC_MSG_RESULT(disabled by user)
fi

])
dnl -- End custom cpu detection autoconf macro
