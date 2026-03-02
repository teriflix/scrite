# The following project include assumes that crashpad has been setup as described
# in the README.md file found in this folder.

INCLUDEPATH += $$PWD
HEADERS += $$PWD/crashpadmodule.h
SOURCES += \
    $$PWD/crashpadmodule_common.cpp
FORMS += \
    $$PWD/CrashRecoveryDialog.ui
OTHER_FILES += $$PWD/README.md

# We need to bundle crashpad only if compiled in release mode, otherwise
# we can let it be.
CONFIG(release, debug|release) {

# Uncomment the following line to enable Ctrl+Shift+Alt+R hotkey to
# force crash Scrite and check if Crashpad works.
# CRASHPAD_TEST_ENABLED = yes

contains(QT_ARCH, x86_64) {
CRASHPAD_SDK = $$(SCRITE_CRASHPAD_ROOT)
} else {
CRASHPAD_SDK = $$(SCRITE_CRASHPAD_ROOT)-x86
}
message("CRASHPAD_SDK at $${CRASHPAD_SDK}")

win32 {
    exists($${CRASHPAD_SDK}/lib/client.lib) {
        LIBS += -L$${CRASHPAD_SDK}/lib/ -lcommon -lclient -lutil -lbase -lAdvapi32
        INCLUDEPATH += $${CRASHPAD_SDK}/include $${CRASHPAD_SDK}/include/crashpad

        CRASHPAD_HANDLER.files = $$shell_path($${CRASHPAD_SDK}\\bin\\crashpad_handler.exe)
        CRASHPAD_HANDLER.path = $$shell_path($$DESTDIR)
        QMAKE_EXTRA_TARGETS += CRASHPAD_HANDLER

        DEFINES += CRASHPAD_AVAILABLE

        CONFIG += force_debug_info
        CONFIG += separate_debug_info

        SOURCES += $$PWD/crashpadmodule_win.cpp
    }
}

macx {
    exists($${CRASHPAD_SDK}/lib/libclient.a) {
        LIBS += -L$${CRASHPAD_SDK}/lib/ -lcommon -lclient -lutil -lbase -lmig_output
        LIBS += -L/usr/lib -lbsm -framework AppKit -framework Security
        INCLUDEPATH += $${CRASHPAD_SDK}/include $${CRASHPAD_SDK}/include/crashpad $${CRASHPAD_SDK}/include/mini_chromium

        CRASHPAD_HANDLER.files = $${CRASHPAD_SDK}/bin/crashpad_handler
        CRASHPAD_HANDLER.path = Contents/MacOS
        QMAKE_BUNDLE_DATA += CRASHPAD_HANDLER

        exists($${CRASHPAD_SDK}/bin/dump_syms) {
            QMAKE_POST_LINK += "$${CRASHPAD_SDK}/bin/dump_syms $$DESTDIR/Scrite.app.dSYM/Contents/Resources/DWARF/Scrite > $$DESTDIR/Scrite.app.sym"
        }

        DEFINES += CRASHPAD_AVAILABLE

        CONFIG += force_debug_info
        CONFIG += separate_debug_info

        SOURCES += $$PWD/crashpadmodule_mac.cpp

        message("Linking with Crashpad for macOS")
    }
}

linux {
    # TODO
}

equals(CRASHPAD_TEST_ENABLED, "yes") {
DEFINES += ENABLE_CRASHPAD_CRASH_TEST
message("Crashpad testing is enabled. Please disable it for release and distributable builds.")
}

}

