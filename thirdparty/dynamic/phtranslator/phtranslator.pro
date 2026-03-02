TEMPLATE = lib
CONFIG += shared
VERSION = 1.1

macx {
    DEFINES += PHTRANSLATE_STATICLIB
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}

linux {
    DEFINES += PHTRANSLATE_STATICLIB
}

HEADERS +=  LanguageCodes.h \
            PhTranslateLib.h \
            PhTranslator.h \
            stdafx.h \
            targetver.h 

SOURCES +=  PhTranslateLib.cpp \
            PhTranslator.cpp \
            stdafx.cpp

DESTDIR = $$PWD/../../../binary
