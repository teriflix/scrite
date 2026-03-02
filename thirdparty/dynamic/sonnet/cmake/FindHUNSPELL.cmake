# SPDX-FileCopyrightText: 2017 Pino Toscano <pino@kde.org>
# SPDX-License-Identifier: BSD-3-Clause
#
# - Try to find HUNSPELL
# Once done this will define
#
#  HUNSPELL_FOUND - system has HUNSPELL
#  HUNSPELL_INCLUDE_DIRS - the HUNSPELL include directory
#  HUNSPELL_LIBRARIES - The libraries needed to use HUNSPELL

find_package(PkgConfig)
pkg_check_modules(PKG_HUNSPELL QUIET hunspell)

find_path(HUNSPELL_INCLUDE_DIRS
          NAMES hunspell.hxx
          PATH_SUFFIXES hunspell
          HINTS ${PKG_HUNSPELL_INCLUDE_DIRS}
)
find_library(HUNSPELL_LIBRARIES
             NAMES ${PKG_HUNSPELL_LIBRARIES} hunspell hunspell-1.6 hunspell-1.5 hunspell-1.4 hunspell-1.3 hunspell-1.2 libhunspell
             HINTS ${PKG_HUNSPELL_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HUNSPELL
                                  REQUIRED_VARS HUNSPELL_LIBRARIES HUNSPELL_INCLUDE_DIRS
                                  VERSION_VAR PKG_HUNSPELL_VERSION
)

mark_as_advanced(HUNSPELL_INCLUDE_DIRS HUNSPELL_LIBRARIES)
