QT += core

macx: LIBS += -lz
win32 {
    DEFINES += QUAZIP_STATIC
    INCLUDEPATH += $$[QT_INSTALL_HEADERS]/QtZlib
}
linux-g++: LIBS += -lz

INCLUDEPATH += $$PWD/quazip

HEADERS += \
    $$PWD/quazip/JlCompress.h \
    $$PWD/quazip/ioapi.h \
    $$PWD/quazip/minizip_crypt.h \
    $$PWD/quazip/quaadler32.h \
    $$PWD/quazip/quachecksum32.h \
    $$PWD/quazip/quacrc32.h \
    $$PWD/quazip/quagzipfile.h \
    $$PWD/quazip/quaziodevice.h \
    $$PWD/quazip/quazip.h \
    $$PWD/quazip/quazip_global.h \
    $$PWD/quazip/quazip_qt_compat.h \
    $$PWD/quazip/quazipdir.h \
    $$PWD/quazip/quazipfile.h \
    $$PWD/quazip/quazipfileinfo.h \
    $$PWD/quazip/quazipnewinfo.h \
    $$PWD/quazip/unzip.h \
    $$PWD/quazip/zip.h

SOURCES += \
    $$PWD/quazip/JlCompress.cpp \
    $$PWD/quazip/qioapi.cpp \
    $$PWD/quazip/quaadler32.cpp \
    $$PWD/quazip/quachecksum32.cpp \
    $$PWD/quazip/quacrc32.cpp \
    $$PWD/quazip/quagzipfile.cpp \
    $$PWD/quazip/quaziodevice.cpp \
    $$PWD/quazip/quazip.cpp \
    $$PWD/quazip/quazipdir.cpp \
    $$PWD/quazip/quazipfile.cpp \
    $$PWD/quazip/quazipfileinfo.cpp \
    $$PWD/quazip/quazipnewinfo.cpp \
    $$PWD/quazip/unzip.c \
    $$PWD/quazip/zip.c
