set(SCRITE_DESKTOP_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DesktopQmlSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DesktopPlatformSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DesktopGenericSources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DesktopQmlAndAssetFiles.cmake")

find_package(Qt6 6.11 REQUIRED COMPONENTS
  Core
  Gui
  Qml
  Quick
  Widgets
  Xml
  Concurrent
  Network
  QuickControls2
  Multimedia
  PrintSupport
  Svg
  Charts
  Pdf
  WebEngineQuick
  Core5Compat
)

if(APPLE)
  enable_language(OBJCXX)
  qt_add_executable(Scrite MACOSX_BUNDLE)
elseif(WIN32)
  qt_add_executable(Scrite WIN32)
else()
  qt_add_executable(Scrite)
endif()

set_source_files_properties(${SCRITE_DESKTOP_SINGLETON_QML_FILES}
  PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(Scrite
  URI io.scrite.components
  RESOURCE_PREFIX "/"
  VERSION 1.0
  QML_FILES
    ${SCRITE_DESKTOP_QML_FILES}
    ${SCRITE_DESKTOP_SINGLETON_QML_FILES}
  SOURCES
    ${SCRITE_DESKTOP_QML_SOURCES}
  RESOURCES
    ${SCRITE_DESKTOP_ICON_FILES}
    ${SCRITE_DESKTOP_IMAGE_FILES}
)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DesktopMiscAndFontsResources.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DesktopIconsAndImages.cmake")

target_sources(Scrite PRIVATE
  ${SCRITE_DESKTOP_SOURCES}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/crashpad/CrashRecoveryDialog.ui"
)

target_compile_definitions(Scrite PRIVATE
  SCRITE_VERSION="${PROJECT_VERSION}"
  SCRITE_VERSION_TYPE="${SCRITE_VERSION_TYPE}"
  SCRITE_QML_URI="io.scrite.components"
  $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

target_include_directories(Scrite PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/../apikeys"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/network"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/importers"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/exporters"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/printing"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/quick"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/quick/objects"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/quick/items"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/utils"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/document"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/interfaces"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/reports"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/restapikey"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/crashpad"
  "${CMAKE_SOURCE_DIR}/thirdparty/static/quazip/quazip/quazip"
  "${CMAKE_SOURCE_DIR}/thirdparty/static/openxlsx/openxlsx/OpenXLSX/headers"
  "${SCRITE_SONNET_INCLUDE_ROOT}"
  "${SCRITE_SONNET_BUILD_INCLUDE_ROOT}"
  "${SCRITE_SONNET_BUILD_CORE_INCLUDE_DIR}"
)

if(EXISTS "${CMAKE_SOURCE_DIR}/profilingtools/timeprofiler.cpp")
  target_include_directories(Scrite PRIVATE "${CMAKE_SOURCE_DIR}/profilingtools")
  target_sources(Scrite PRIVATE
    "${CMAKE_SOURCE_DIR}/profilingtools/timeprofiler.cpp"
    "${CMAKE_SOURCE_DIR}/profilingtools/timeprofiler.h"
  )
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/profilingtools/callgraph.cpp")
  target_include_directories(Scrite PRIVATE "${CMAKE_SOURCE_DIR}/profilingtools")
  target_sources(Scrite PRIVATE
    "${CMAKE_SOURCE_DIR}/profilingtools/callgraph.cpp"
    "${CMAKE_SOURCE_DIR}/profilingtools/callgraph.h"
  )
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/apikeys")
  target_include_directories(Scrite PRIVATE "${CMAKE_SOURCE_DIR}/apikeys")
endif()

target_link_libraries(Scrite PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Qml
  Qt6::Quick
  Qt6::Widgets
  Qt6::Xml
  Qt6::Concurrent
  Qt6::Network
  Qt6::QuickControls2
  Qt6::Multimedia
  Qt6::PrintSupport
  Qt6::Svg
  Qt6::Charts
  Qt6::Pdf
  Qt6::WebEngineQuick
  Qt6::Core5Compat

  QuaZip::QuaZip
  OpenXLSX
  poly2tri
  simplecrypt
  phtranslator
  KF6::SonnetCore
)

if(APPLE)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
                 @ONLY)

  set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/appicon.icns"
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources
  )
  target_sources(Scrite PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/appicon.icns")

  set_target_properties(Scrite PROPERTIES
    OSX_ARCHITECTURES "x86_64;arm64"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
  )
  target_link_libraries(Scrite PRIVATE "-framework Carbon")

  # Copy dynamic libraries and the kf6 plugins folder into the app bundle.
  # Dylibs go into Contents/Frameworks/ (standard macOS layout).
  # Plugins go into Contents/PlugIns/ (Qt convention).
  add_custom_command(TARGET Scrite POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory
            "$<TARGET_FILE_DIR:Scrite>/../Frameworks"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:KF6SonnetCore>"
            "$<TARGET_FILE_DIR:Scrite>/../Frameworks/$<TARGET_FILE_NAME:KF6SonnetCore>"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:phtranslator>"
            "$<TARGET_FILE_DIR:Scrite>/../Frameworks/$<TARGET_FILE_NAME:phtranslator>"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${CMAKE_SOURCE_DIR}/binary/kf6"
            "$<TARGET_FILE_DIR:Scrite>/../PlugIns/kf6"
    COMMENT "Copying KF6SonnetCore, phtranslator, and kf6 plugins into Scrite.app"
    VERBATIM
  )

  # Set rpath so the executable finds dylibs in Contents/Frameworks/
  set_target_properties(Scrite PROPERTIES
    INSTALL_RPATH "@executable_path/../Frameworks"
    BUILD_RPATH "$<TARGET_FILE_DIR:KF6SonnetCore>;$<TARGET_FILE_DIR:phtranslator>"
  )
endif()

if(WIN32)
  target_sources(Scrite PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")
  target_link_libraries(Scrite PRIVATE User32)
  target_compile_options(Scrite PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
  )
endif()

if(UNIX AND NOT APPLE)
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(IBUS QUIET ibus-1.0)
    if(IBUS_FOUND)
      target_include_directories(Scrite PRIVATE ${IBUS_INCLUDE_DIRS})
      target_link_libraries(Scrite PRIVATE ${IBUS_LIBRARIES})
      target_compile_options(Scrite PRIVATE ${IBUS_CFLAGS_OTHER})
    endif()
  endif()
endif()

# Optional crashpad integration (off by default).
option(SCRITE_WITH_CRASHPAD "Enable Crashpad integration (requires SDK)" OFF)
set(SCRITE_CRASHPAD_ROOT "$ENV{SCRITE_CRASHPAD_ROOT}" CACHE PATH "Crashpad SDK root (SCRITE_CRASHPAD_ROOT)")

if(SCRITE_WITH_CRASHPAD AND SCRITE_CRASHPAD_ROOT)
  if(WIN32)
    if(EXISTS "${SCRITE_CRASHPAD_ROOT}/lib/client.lib")
      target_compile_definitions(Scrite PRIVATE CRASHPAD_AVAILABLE)
      target_include_directories(Scrite PRIVATE
        "${SCRITE_CRASHPAD_ROOT}/include"
        "${SCRITE_CRASHPAD_ROOT}/include/crashpad"
      )
      target_link_directories(Scrite PRIVATE "${SCRITE_CRASHPAD_ROOT}/lib")
      target_link_libraries(Scrite PRIVATE common client util base Advapi32)
      target_sources(Scrite PRIVATE ${SCRITE_DESKTOP_WIN_CRASHPAD_PLATFORM_SOURCES})
      add_custom_command(TARGET Scrite POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${SCRITE_CRASHPAD_ROOT}/bin/crashpad_handler.exe"
                "$<TARGET_FILE_DIR:Scrite>/crashpad_handler.exe"
      )
    endif()
  elseif(APPLE)
    if(EXISTS "${SCRITE_CRASHPAD_ROOT}/lib/libclient.a")
      target_compile_definitions(Scrite PRIVATE CRASHPAD_AVAILABLE)
      target_include_directories(Scrite PRIVATE
        "${SCRITE_CRASHPAD_ROOT}/include"
        "${SCRITE_CRASHPAD_ROOT}/include/crashpad"
        "${SCRITE_CRASHPAD_ROOT}/include/mini_chromium"
      )
      target_link_directories(Scrite PRIVATE "${SCRITE_CRASHPAD_ROOT}/lib")
      target_link_libraries(Scrite PRIVATE common client util base mig_output bsm "-framework AppKit" "-framework Security")
      target_sources(Scrite PRIVATE ${SCRITE_DESKTOP_MAC_CRASHPAD_PLATFORM_SOURCES})
      add_custom_command(TARGET Scrite POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${SCRITE_CRASHPAD_ROOT}/bin/crashpad_handler"
                "$<TARGET_FILE_DIR:Scrite>/crashpad_handler"
      )
    endif()
    foreach(_var
      CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS
      CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      CMAKE_SHARED_MODULE_CREATE_C_FLAGS
      CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS
    )
      if(DEFINED ${_var})
        string(REPLACE "-Wl,-single_module" "" ${_var} "${${_var}}")
        string(REPLACE "-single_module" "" ${_var} "${${_var}}")
        string(REPLACE "-Wl,-multiply_defined,suppress" "" ${_var} "${${_var}}")
        string(REPLACE "-multiply_defined suppress" "" ${_var} "${${_var}}")
      endif()
    endforeach()
  endif()
endif()

# Force rebuild of application_build_timestamp.cpp on every build.
add_custom_target(scrite_touch_build_timestamp ALL
  COMMAND "${CMAKE_COMMAND}" -E touch
          "${SCRITE_DESKTOP_SRC_DIR}/core/application_build_timestamp.cpp"
)
add_dependencies(Scrite scrite_touch_build_timestamp)

# CPack install rules
if(APPLE)
  install(TARGETS Scrite BUNDLE DESTINATION .)
elseif(WIN32)
  install(TARGETS Scrite RUNTIME DESTINATION .)
elseif(UNIX)
  install(TARGETS Scrite RUNTIME DESTINATION bin)
  install(FILES "${CMAKE_SOURCE_DIR}/packaging/assets/linux/Scrite.desktop"
          DESTINATION share/applications)
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/images/appicon.png"
          DESTINATION share/icons/hicolor/512x512/apps
          RENAME Scrite.png)
endif()

