TEMPLATE = lib
CONFIG += static
DESTDIR = $$PWD/../../../binary

macx: QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64

HEADERS += simplecrypt.h
SOURCES += simplecrypt.cpp

