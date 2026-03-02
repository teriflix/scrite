TEMPLATE = lib
CONFIG += shared
DESTDIR = $$PWD/../../../../../binary/

QT += core

VERSION = 6.23.0
DEFINES += ScriteSonnetCore_VERSION_STR=\\\"$$VERSION\\\" \
           ScriteSonnetCore_VERSION_MAJOR=6 \
           ScriteSonnetCore_VERSION_MINOR=23 \
           ScriteSonnetCore_VESION_REVISION=0

TARGET = sonnet
DEFINES += ScriteSonnetCore_EXPORTS SONNET_STATIC

macx: QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

HEADERS +=  $$PWD/backgroundchecker_p.h \
            $$PWD/backgroundchecker.h \
            $$PWD/client_p.h \
            $$PWD/guesslanguage.h \
            $$PWD/languagefilter_p.h \
            $$PWD/loader_p.h \
            $$PWD/settings.h \
            $$PWD/settingsimpl_p.h \
            $$PWD/speller.h \
            $$PWD/spellerplugin_p.h \
            $$PWD/textbreaks_p.h \
            $$PWD/tokenizer_p.h

SOURCES +=  $$PWD/backgroundchecker.cpp \
            $$PWD/client.cpp \
            $$PWD/guesslanguage.cpp \
            $$PWD/languagefilter.cpp \
            $$PWD/loader.cpp \
            $$PWD/settings.cpp \
            $$PWD/settingsimpl.cpp \
            $$PWD/speller.cpp \
            $$PWD/spellerplugin.cpp \
            $$PWD/textbreaks.cpp \
            $$PWD/tokenizer.cpp

# The following would have been auto-generated had we used CMake, but
# we need to manually generate this and bake it into the project file.
HEADERS += $$PWD/core_debug.h $$PWD/sonnet_version.h $$PWD/sonnetcore_export.h
SOURCES += $$PWD/core_debug.cpp
