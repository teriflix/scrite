macx {
    LIBS += -lz
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

INCLUDEPATH += $$PWD $$PWD/quazip

LIBS += -L$$PWD/../../../binary/ -lquazip
