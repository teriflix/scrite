include(CheckIncludeFile)

# needs windows 8 or higher
add_definitions(-DWINVER=0x0602 -D_WIN32_WINNT=0x0602)

CHECK_INCLUDE_FILE(spellcheck.h HAS_SPELLCHECK_H)

if (NOT HAS_SPELLCHECK_H)
    return()
endif()

qt_add_plugin(sonnet_ispellchecker CLASS_NAME ISpellCheckerClient)

target_sources(sonnet_ispellchecker PRIVATE
    ispellcheckerclient.cpp
    ispellcheckerdict.cpp
)

ecm_qt_declare_logging_category(sonnet_ispellchecker
    HEADER ispellcheckerdebug.h
    IDENTIFIER SONNET_ISPELLCHECKER
    CATEGORY_NAME kf.sonnet.clients.ispellchecker
    OLD_CATEGORY_NAMES sonnet.plugins.ispellchecker
    DESCRIPTION "Sonnet ISpellChecker plugin"
    EXPORT SONNET
)

target_link_libraries(sonnet_ispellchecker PRIVATE KF6::SonnetCore)
target_compile_definitions(sonnet_ispellchecker PRIVATE DEFINITIONS SONNET_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

install(TARGETS sonnet_ispellchecker  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/sonnet/)

set(SONNET_BACKEND_FOUND TRUE PARENT_SCOPE)
