AC_PREREQ(2.59)
AC_INIT([Aethyra], [0.0.29.1], [irarice@gmail.com], [aethyra])
AC_CONFIG_HEADERS([config.h:config.h.in])
AC_LANG_CPLUSPLUS

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit floor getcwd gethostbyname memset mkdir select socket])

# Checks for internationalization support
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.16.1])

# Search for *-config
AC_PATH_PROG(SDL_CONFIG, sdl-config)
AC_PATH_PROG(PKG_CONFIG, pkg-config)
AC_PATH_PROG(CURL_CONFIG, curl-config)

# Checks for libraries
AC_CHECK_LIB([pthread], [pthread_create], ,
AC_MSG_ERROR([ *** Unable to find pthread library]))

AC_CHECK_LIB([guichan], [gcnGuichanVersion], ,
AC_MSG_ERROR([ *** Unable to find Guichan library (guichan.sf.net)]))

AC_CHECK_LIB([z], [inflate], ,
AC_MSG_ERROR([ *** Unable to find zlib (http://www.gzip.org/zlib/)]))

AC_CHECK_LIB([physfs], [PHYSFS_init], ,
AC_MSG_ERROR([ *** Unable to find PhysFS library (icculus.org/physfs/)]))

AC_CHECK_LIB([curl], [curl_global_init], ,
AC_MSG_ERROR([ *** Unable to find CURL library (http://curl.haxx.se/)]))
CURL_LIBS=" `curl-config --libs`"
CURL_CFLAGS=" `curl-config --cflags` "
AC_SUBST(CURL_LIBS)
AC_SUBST(CURL_CFLAGS)

AC_CHECK_LIB([xml2], [xmlInitParser], ,
AC_MSG_ERROR([ *** Unable to find libxml2 library (xmlsoft.org)]))

AC_CHECK_LIB(png, png_write_info, ,
AC_MSG_ERROR([ *** Unable to find png library]))

AC_CHECK_LIB(SDL_image, IMG_LoadPNG_RW, ,
AC_MSG_ERROR([ *** Unable to find SDL_image library with PNG support
(http://www.libsdl.org/projects/SDL_image/)]))

AC_CHECK_LIB(SDL_ttf, TTF_Quit, ,
AC_MSG_ERROR([ *** Unable to find SDL_ttf library (http://www.libsdl.org/projects/SDL_ttf/)]))

AC_CHECK_LIB([SDL_mixer], [Mix_OpenAudio], ,
AC_MSG_ERROR([ *** Unable to find SDL_mixer library (http://www.libsdl.org/projects/SDL_mixer/)]))

AC_CHECK_LIB(SDL_net, SDLNet_Init, ,
AC_MSG_ERROR([ *** Unable to find SDL_net library]))

AC_CHECK_LIB(SDL_gfx, rotozoomSurfaceXY, ,
AC_MSG_ERROR([ *** Unable to find SDL_gfx library (http://www.ferzkopp.net/%7Easchiffler/Software/SDL_gfx-2.0/index.html)]))
AC_CHECK_HEADERS(SDL_rotozoom.h, ,)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Check for guichan headers
AC_CHECK_HEADERS([guichan.hpp], ,
AC_MSG_ERROR([*** Library found but cannot find headers (guichan.sf.net) *** ]))

AM_INIT_AUTOMAKE

# Option to enable OpenGL
AC_ARG_WITH(opengl,[  --without-opengl        don't use OpenGL ] )
if test "x$with_opengl" == "xno"; then
    with_opengl=no
else
    with_opengl=yes
    AC_CHECK_LIB([GL], [glBegin], ,
    AC_MSG_ERROR([ *** Unable to find OpenGL library]))
    AC_DEFINE(USE_OPENGL, 1, [Defines if aethyra should use an OpenGL display])
fi

# Check for X11
if test "x$OSX" != "xyes"; then
  AC_CHECK_HEADER(X11/Xlib.h, check_x11=yes, check_x11=no)
  if test "x$check_x11" = "xno"; then
    AC_CHECK_HEADER(X11R6/Xlib.h,
                    [ check_x11=yes
                      LDFLAGS="-L/usr/X11R6/include $CFLAGS"],
                    check_x11=no)
  fi
  if test "x$check_x11" = "xyes"; then
    AC_SEARCH_LIBS(XOpenDisplay, X11, use_x11=yes, use_x11=no, )
    if test "x$use_x11" = "xyes"; then
      AC_DEFINE(USE_X11, 1, [Define to use X11 clipboard])
    fi
  fi
fi

# Search for sdl-config
AC_PATH_PROG(LIBSDL_CONFIG, sdl-config)

if test -n "$LIBSDL_CONFIG"; then
  LIBSDL_LIBS="`$LIBSDL_CONFIG --libs` -lSDL_image -lSDL_mixer -lSDL_ttf"
  LIBSDL_CFLAGS="`$LIBSDL_CONFIG --cflags`"
  AC_SUBST(LIBSDL_LIBS)
  AC_SUBST(LIBSDL_CFLAGS)

  AC_DEFINE_UNQUOTED(HAVE_LIBSDL, 1,
		     [Defines if your system has the LIBSDL library])
else
  AC_DEFINE_UNQUOTED(HAVE_LIBSDL, 0,
		     [Defines if your system has the LIBSDL library])
  AC_MSG_ERROR([Could not find sdl-config, check http://www.libsdl.org])
fi

AC_CONFIG_FILES([
Makefile
src/Makefile
data/Makefile
data/fonts/Makefile
data/graphics/Makefile
data/graphics/gui/Makefile
data/graphics/images/Makefile
data/help/Makefile
data/icons/Makefile
docs/Makefile
po/Makefile.in
])

AC_OUTPUT

echo
echo Build with OpenGL: $with_opengl
echo
echo configure complete, now type \"make\"
echo
