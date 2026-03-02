TEMPLATE = lib
CONFIG += shared plugin

INCLUDEPATH += $$PWD/../../core $$PWD/../../core/misc
LIBS += -L$$PWD/../../../../../../binary/ -lsonnet
DESTDIR = $$PWD/../../../../../../binary/
QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

HEADERS += nsspellcheckerclient.h nsspellcheckerdict.h
OBJECTIVE_SOURCES += nsspellcheckerclient.mm nsspellcheckerdict.mm
LIBS    +=  -framework AppKit

# The following would have been auto-generated had we used CMake, but
# we need to manually generate this and bake it into the project file.
HEADERS += nsspellcheckerdebug.h
SOURCES += nsspellcheckerdebug.cpp
