
# you can find find_package() in kdelibs/cmake/modules/OptionalFindPackage.cmake
# it is the same as FIND_PACKAGE(<name>) but additionally creates an OPTION(WITH_<name>)
# so the checking for the software can be disabled via ccmake or -DWITH_<name>=OFF
if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.1)
    find_package(ASPELL COMPONENTS ASPELL)
else()
    find_package(ASPELL)
endif()
set_package_properties(ASPELL PROPERTIES
    URL "http://aspell.net/"
    DESCRIPTION "Spell checking support via Aspell"
    TYPE OPTIONAL
)
if (ASPELL_INCLUDE_DIR AND ASPELL_LIBRARIES)
   add_subdirectory( aspell )
endif ()


find_package(HSPELL)
set_package_properties(HSPELL PROPERTIES
    URL "http://ivrix.org.il/projects/spell-checker/"
    DESCRIPTION "Spell checking support for Hebrew"
    TYPE OPTIONAL
)
if (HSPELL_FOUND)
   add_subdirectory( hspell )
endif ()

find_package(HUNSPELL)
set_package_properties(HUNSPELL PROPERTIES
    URL "https://hunspell.github.io/"
    DESCRIPTION "Spell checking support via Hunspell"
    TYPE OPTIONAL
)
if (HUNSPELL_FOUND)
   add_subdirectory( hunspell )
endif ()


find_package(VOIKKO)
set_package_properties(VOIKKO PROPERTIES
    URL "https://voikko.puimula.org/"
    DESCRIPTION "Spell checking support via Voikko"
    TYPE OPTIONAL
)
if (VOIKKO_FOUND)
   add_subdirectory( voikko )
endif ()

if (APPLE)
    add_subdirectory( nsspellchecker )
endif ()

if (WIN32)
    add_subdirectory( ispellchecker )
endif ()

# if we did not find any backend, that is bad
# do that on Android, too, if we have some backend for it
if (NOT ANDROID AND NOT SONNET_BACKEND_FOUND AND NOT SONNET_NO_BACKENDS)
    message(FATAL_ERROR "Can not build any backend plugin for Sonnet.")
endif ()
