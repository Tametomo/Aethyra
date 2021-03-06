CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(AETHYRA)

SET(VERSION 0.0.29.1)

# where to look for cmake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)

FIND_PACKAGE(Gettext)

OPTION(WITH_OPENGL "Enable OpenGL support" ON)
OPTION(ENABLE_NLS "Enable building of translations" ON)

IF (WIN32)
    SET(PKG_DATADIR ".")
    SET(PKG_BINDIR ".")
    SET(LOCALEDIR ".")
ELSE (WIN32)
    IF (NOT OSX)
        OPTION(USE_X11 "Use X11 Clipboard functionality" ON)
    ENDIF ()
    SET(PKG_DATADIR ${CMAKE_INSTALL_PREFIX}/share/aethyra)
    SET(LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)
    SET(PKG_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
ENDIF (WIN32)

ADD_SUBDIRECTORY(data)
ADD_SUBDIRECTORY(src)

IF (GETTEXT_FOUND AND ENABLE_NLS)
    ADD_SUBDIRECTORY(po)
ELSE (GETTEXT_FOUND AND ENABLE_NLS)
    MESSAGE(STATUS "Gettext not found. Compilation will not generate translations.")
ENDIF()

INSTALL(FILES aethyra.desktop DESTINATION share/applications)
