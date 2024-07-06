# The following project include assumes that crashpad has been setup as described
# in the README.md file found in this folder.

HEADERS += $$PWD/crashpadmodule.h
SOURCES += \
    $$PWD/crashpadmodule_common.cpp
FORMS += \
    $$PWD/CrashRecoveryDialog.ui

# We need to bundle crashpad only if compiled in release mode, otherwise
# we can let it be.
CONFIG(release, debug|release) {

# Uncomment the following line to enable Ctrl+Shift+Alt+R hotkey to
# force crash Scrite and check if Crashpad works.
# CRASHPAD_TEST_ENABLED = yes

win32 {
    # contains(QT_ARCH, i386) {
    #     SCRITE_CRASHPAD_ROOT = $$(SCRITE_CRASHPAD_ROOT)/x86/MD
    # } else {
    #     SCRITE_CRASHPAD_ROOT = $$(SCRITE_CRASHPAD_ROOT)/x64/MD
    # }

    CRASHPAD_SDK = $$(SCRITE_CRASHPAD_ROOT)

    exists($${CRASHPAD_SDK}/lib/client.lib) {
        LIBS += -L$${CRASHPAD_SDK}/lib/ -lcommon -lclient -lutil -lbase -lAdvapi32
        INCLUDEPATH += $${CRASHPAD_SDK}/include $${CRASHPAD_SDK}/include/crashpad

        CRASHPAD_HANDLER.files = $$shell_path($${CRASHPAD_SDK}\\bin\\crashpad_handler.exe)
        CRASHPAD_HANDLER.path = $$shell_path($$DESTDIR)
        QMAKE_EXTRA_TARGETS += CRASHPAD_HANDLER

        DEFINES += CRASHPAD_AVAILABLE

        # Create symbols for dump_syms and symupload
        CONFIG += force_debug_info
        CONFIG += separate_debug_info

        # Build Crashpad Support for Scrite
        INCLUDEPATH += $$PWD
        SOURCES += $$PWD/crashpadmodule_win.cpp
    }
}

macx {
    # TODO
}

linux {
    # TODO
}

equals(CRASHPAD_TEST_ENABLED, "yes") {
DEFINES += ENABLE_CRASHPAD_CRASH_TEST
message("Crashpad testing is enabled. Please disable it for release and distributable builds.")
}

}

