# macOS CPack configuration — produces a drag-and-drop DMG installer.
#
# The package-macos.sh script runs macdeployqt and generates the DMG background
# image before invoking cpack, so background.png is expected to already exist
# at packaging/assets/mac/background.png by the time cpack runs.

set(CPACK_GENERATOR "DragNDrop")

set(CPACK_DMG_VOLUME_NAME "Scrite ${PROJECT_VERSION}")
set(CPACK_DMG_FORMAT "ULFO")
set(CPACK_DMG_BACKGROUND_IMAGE
    "${CMAKE_SOURCE_DIR}/packaging/assets/mac/background.png")

set(CPACK_PACKAGE_FILE_NAME "Scrite-${PROJECT_VERSION}")

# Install the app bundle into the root of the DMG staging area.
install(TARGETS Scrite BUNDLE DESTINATION .)

# Install LICENSE.txt to the root of the DMG
install(FILES "${CMAKE_SOURCE_DIR}/packaging/assets/mac/license.txt" DESTINATION .)
