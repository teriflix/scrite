# Windows CPack configuration — produces an NSIS installer.
#
# The package-windows.sh script builds a staging directory (populated by
# windeployqt) and passes it via SCRITE_STAGING_DIR before invoking cpack.
# CPack uses CPACK_INSTALLED_DIRECTORIES to package the pre-built staging dir
# rather than relying on install() rules (because windeployqt generates the
# file list dynamically at runtime).

set(CPACK_GENERATOR "NSIS")

# Include version suffix if provided (e.g., SCRITE_VERSION_SUFFIX="-beta")
if(DEFINED SCRITE_VERSION_SUFFIX)
    set(CPACK_PACKAGE_FILE_NAME "Scrite-${PROJECT_VERSION}${SCRITE_VERSION_SUFFIX}-64bit-Setup")
else()
    set(CPACK_PACKAGE_FILE_NAME "Scrite-${PROJECT_VERSION}-64bit-Setup")
endif()
set(CPACK_NSIS_PACKAGE_NAME "Scrite")
set(CPACK_NSIS_DISPLAY_NAME "Scrite ${PROJECT_VERSION}")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

# Icons
set(CPACK_NSIS_MUI_ICON
    "${CMAKE_SOURCE_DIR}/apps/desktop/appicon.ico")
set(CPACK_NSIS_MUI_UNIICON
    "${CMAKE_SOURCE_DIR}/apps/desktop/appicon.ico")

# Shortcuts
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$DESKTOP\\\\Scrite.lnk' '$INSTDIR\\\\Scrite.exe'")
set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$DESKTOP\\\\Scrite.lnk'")

# Launch Scrite from finish page
set(CPACK_NSIS_MUI_FINISHPAGE_RUN "Scrite.exe")

# Publisher / URL shown in Add/Remove Programs
set(CPACK_NSIS_URL_INFO_ABOUT "https://www.scrite.io")
set(CPACK_NSIS_CONTACT "support@scrite.io")

# Register .scrite file extension and notify shell on install/uninstall.
# FileAssociation.nsh must be present alongside the generated .nsi script;
# package-windows.sh copies it to the staging dir before invoking cpack.
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS [[
  !include "FileAssociation.nsh"
  ${registerExtension} "$INSTDIR\Scrite.exe" ".scrite" "Scrite Screenplay Document"
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'
  ExecWait "$INSTDIR\vcredist_x64.exe /q"
]])

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS [[
  !include "FileAssociation.nsh"
  ${unregisterExtension} ".scrite" "Scrite Screenplay Document"
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'
]])

# Use the staging directory pre-populated by windeployqt.
# SCRITE_STAGING_DIR must be passed as a cmake -D argument when invoking cpack.
if(DEFINED SCRITE_STAGING_DIR AND EXISTS "${SCRITE_STAGING_DIR}")
    set(CPACK_INSTALLED_DIRECTORIES "${SCRITE_STAGING_DIR};.")
else()
    message(WARNING
        "SCRITE_STAGING_DIR not set or does not exist. "
        "Run package-windows.sh instead of invoking cpack directly.")
    install(TARGETS Scrite RUNTIME DESTINATION .)
endif()
