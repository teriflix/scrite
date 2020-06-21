DEFINES += SONNETUI_EXPORT=""
DEFINES += SONNETCORE_EXPORT=""
DEFINES += INSTALLATION_PLUGIN_PATH=""
DEFINES += SONNET_STATIC

INCLUDEPATH += $$PWD \
               $$PWD/sonnet/src/core/ \
               $$PWD/sonnet/src/plugins/ \
               $$PWD/sonnet/src/plugins/nsspellchecker

HEADERS +=  $$PWD/sonnetcore_export.h \
            $$PWD/core_debug.h \
            $$PWD/spellcheckservice.h

HEADERS +=  $$PWD/sonnet/src/core/client_p.h \
            $$PWD/sonnet/src/core/tokenizer_p.h \
            $$PWD/sonnet/src/core/backgroundchecker_p.h \
            $$PWD/sonnet/src/core/spellerplugin_p.h \
            $$PWD/sonnet/src/core/textbreaks_p.h \
            $$PWD/sonnet/src/core/languagefilter_p.h \
            $$PWD/sonnet/src/core/speller.h \
            $$PWD/sonnet/src/core/settings_p.h \
            $$PWD/sonnet/src/core/guesslanguage.h \
            $$PWD/sonnet/src/core/backgroundchecker.h \
            $$PWD/sonnet/src/core/loader_p.h

SOURCES +=  $$PWD/core_debug.cpp \
    $$PWD/spellcheckservice.cpp

SOURCES +=  $$PWD/sonnet/src/core/client.cpp \
            $$PWD/sonnet/src/core/spellerplugin.cpp \
            $$PWD/sonnet/src/core/textbreaks.cpp \
            $$PWD/sonnet/src/core/backgroundchecker.cpp \
            $$PWD/sonnet/src/core/tokenizer.cpp \
            $$PWD/sonnet/src/core/guesslanguage.cpp \
            $$PWD/sonnet/src/core/languagefilter.cpp \
            $$PWD/sonnet/src/core/speller.cpp \
            $$PWD/sonnet/src/core/loader.cpp \
            $$PWD/sonnet/src/core/settings.cpp

HEADERS  += $$PWD/plugins/dummy/dummyclient.h

SOURCES  += $$PWD/plugins/dummy/dummyclient.cpp

macx {
HEADERS +=  $$PWD/nsspellcheckerdebug.h \
            $$PWD/sonnet/src/plugins/nsspellchecker/nsspellcheckerdict.h \
            $$PWD/sonnet/src/plugins/nsspellchecker/nsspellcheckerclient.h
SOURCES +=  $$PWD/nsspellcheckerdebug.cpp
OBJECTIVE_SOURCES += $$PWD/sonnet/src/plugins/nsspellchecker/nsspellcheckerdict.mm \
            $$PWD/sonnet/src/plugins/nsspellchecker/nsspellcheckerclient.mm
LIBS    +=  -framework AppKit
DEFINES += SONNET_AVOID_HUNSPELL_CLIENT # We use a native NSSpellChecker from AppKit
}

win32 {
HEADERS += $$PWD/plugins/windows/windowsclient.h
SOURCES += $$PWD/plugins/windows/windowsclient.cpp
LIBS    += Ole32.lib
DEFINES += SONNET_AVOID_HUNSPELL_CLIENT # We use ISpellChecker from Windows SDK
}

linux-g++ {
HEADERS +=  $$PWD/hunspelldebug.h \
            $$PWD/config-hunspell.h \
            $$PWD/sonnet/src/plugins/hunspell/hunspellclient.h \
            $$PWD/sonnet/src/plugins/hunspell/hunspelldict.h
SOURCES +=  $$PWD/hunspelldebug.cpp \
            $$PWD/sonnet/src/plugins/hunspell/hunspellclient.cpp \
            $$PWD/sonnet/src/plugins/hunspell/hunspelldict.cpp
LIBS    += -lhunspell
INCLUDEPATH += /usr/include/hunspell
}
