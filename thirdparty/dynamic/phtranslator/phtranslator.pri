macx {
    DEFINES += PHTRANSLATE_STATICLIB
}
linux {
    DEFINES += PHTRANSLATE_STATICLIB
}

INCLUDEPATH += $$PWD
LIBS += -L$$PWD/../../../binary/ -lphtranslator
