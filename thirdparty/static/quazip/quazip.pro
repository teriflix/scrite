TEMPLATE = lib
CONFIG += static
DESTDIR = $$PWD/../../../binary

QT += core core5compat

macx {
    LIBS += -lz
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}

linux {
    LIBS += -lz
}

win32 {
    DEFINES += QUAZIP_STATIC
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
}

DEFINES += ZLIB_CONST QUAZIP_CAN_USE_QTEXTCODEC

contains(LIBS, -lbz2) {
} else {
    LIBS += -lbz2
}

INCLUDEPATH += $$PWD/quazip

HEADERS += \
    quazip/ioapi.h \
    quazip/JlCompress.h \
    quazip/minizip_crypt.h \
    quazip/quaadler32.h \
    quazip/quachecksum32.h \
    quazip/quacrc32.h \
    quazip/quagzipfile.h \
    quazip/quaziodevice.h \
    quazip/quazip_global.h \
    quazip/quazip_qt_compat.h \
    quazip/quazip_textcodec.h \
    quazip/quazip.h \
    quazip/quazipdir.h \
    quazip/quazipfile.h \
    quazip/quazipfileinfo.h \
    quazip/quazipnewinfo.h \
    quazip/unzip.h \
    quazip/zip.h 

SOURCES += \
    quazip/JlCompress.cpp \
    quazip/qioapi.cpp \
    quazip/quaadler32.cpp \
    quazip/quachecksum32.cpp \
    quazip/quacrc32.cpp \
    quazip/quagzipfile.cpp \
    quazip/quaziodevice.cpp \
    quazip/quazip_textcodec.cpp \
    quazip/quazip.cpp \
    quazip/quazipdir.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipfileinfo.cpp \
    quazip/quazipnewinfo.cpp
