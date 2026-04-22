# Linux CPack configuration — produces an AppImage.
#
# CPack stages the install tree; then the external packaging script
# (_appimage_pack.cmake) calls linuxdeployqt / appimagetool to create the
# final .AppImage from that staged tree.

set(CPACK_GENERATOR "External")
set(CPACK_EXTERNAL_ENABLE_STAGING TRUE)
set(CPACK_EXTERNAL_PACKAGE_SCRIPT
    "${CMAKE_SOURCE_DIR}/packaging/scripts/_appimage_pack.cmake")

set(CPACK_PACKAGE_FILE_NAME "Scrite-${PROJECT_VERSION}-x86_64")

# Standard FHS install layout for the AppDir
install(TARGETS Scrite RUNTIME DESTINATION bin)

install(FILES
    "${CMAKE_SOURCE_DIR}/packaging/assets/linux/Scrite.desktop"
    DESTINATION share/applications)

install(FILES
    "${CMAKE_SOURCE_DIR}/apps/desktop/images/appicon.png"
    DESTINATION share/icons/hicolor/512x512/apps
    RENAME Scrite.png)
