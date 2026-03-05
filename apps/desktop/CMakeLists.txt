set(SCRITE_DESKTOP_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE SCRITE_DESKTOP_CPP CONFIGURE_DEPENDS
  "${SCRITE_DESKTOP_SRC_DIR}/*.cpp"
)
file(GLOB_RECURSE SCRITE_DESKTOP_H CONFIGURE_DEPENDS
  "${SCRITE_DESKTOP_SRC_DIR}/*.h"
)
file(GLOB_RECURSE SCRITE_DESKTOP_MM CONFIGURE_DEPENDS
  "${SCRITE_DESKTOP_SRC_DIR}/*.mm"
)

# Exclude files that qmake explicitly excluded or are platform-specific.
list(REMOVE_ITEM SCRITE_DESKTOP_CPP
  "${SCRITE_DESKTOP_SRC_DIR}/reports/locationscreenplayreport.cpp"
  "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_linux.cpp"
  "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_windows.cpp"
  "${SCRITE_DESKTOP_SRC_DIR}/crashpad/crashpadmodule_mac.cpp"
  "${SCRITE_DESKTOP_SRC_DIR}/crashpad/crashpadmodule_win.cpp"
)
list(REMOVE_ITEM SCRITE_DESKTOP_H
  "${SCRITE_DESKTOP_SRC_DIR}/reports/locationscreenplayreport.h"
  "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_linux.h"
  "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_windows.h"
  "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_macos.h"
)
list(REMOVE_ITEM SCRITE_DESKTOP_MM
  "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_macos.mm"
)

set(SCRITE_DESKTOP_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
  ${SCRITE_DESKTOP_CPP}
  ${SCRITE_DESKTOP_H}
)

set(SCRITE_DESKTOP_RESOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/ui.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/misc/misc.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/images/images.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/icons/icons.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Bengali/bengali_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/English/english_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Gujarati/gujarati_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Hindi/hindi_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Kannada/kannada_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Malayalam/malayalam_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Marathi/marathi_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Oriya/oriya_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Punjabi/punjabi_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Rubik/rubik_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Sanskrit/sanskrit_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Tamil/tamil_font.qrc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Telugu/telugu_font.qrc"
)

find_package(Qt6 6.8 REQUIRED COMPONENTS
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
else()
  qt_add_executable(Scrite)
endif()

target_sources(Scrite PRIVATE
  ${SCRITE_DESKTOP_SOURCES}
  ${SCRITE_DESKTOP_MM}
  ${SCRITE_DESKTOP_RESOURCES}
  "${CMAKE_CURRENT_SOURCE_DIR}/src/crashpad/CrashRecoveryDialog.ui"
)

target_compile_definitions(Scrite PRIVATE
  SCRITE_VERSION="2.0.21"
  SCRITE_VERSION_TYPE=""
  SCRITE_QML_URI="io.scrite.components"
  $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

target_include_directories(Scrite PRIVATE
  "${CMAKE_SOURCE_DIR}"
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
  poly2tri
  simplecrypt
  phtranslator
  "$<TARGET_FILE:KF6SonnetCore>"
)

if(APPLE)
  set_target_properties(Scrite PROPERTIES
    OSX_ARCHITECTURES "x86_64;arm64"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
  )
  target_link_libraries(Scrite PRIVATE "-framework Carbon")
endif()

if(WIN32)
  target_link_libraries(Scrite PRIVATE User32)
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

if(APPLE)
  # Only include this on macOS; it requires Objective-C++.
  target_sources(Scrite PRIVATE
    "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_macos.mm"
    "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_macos.h"
  )
elseif(WIN32)
  target_sources(Scrite PRIVATE
    "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_windows.cpp"
    "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_windows.h"
  )
else()
  target_sources(Scrite PRIVATE
    "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_linux.cpp"
    "${SCRITE_DESKTOP_SRC_DIR}/core/platformtransliterator_linux.h"
  )
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
      target_sources(Scrite PRIVATE "${SCRITE_DESKTOP_SRC_DIR}/crashpad/crashpadmodule_win.cpp")
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
      target_sources(Scrite PRIVATE "${SCRITE_DESKTOP_SRC_DIR}/crashpad/crashpadmodule_mac.cpp")
      add_custom_command(TARGET Scrite POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${SCRITE_CRASHPAD_ROOT}/bin/crashpad_handler"
                "$<TARGET_FILE_DIR:Scrite>/crashpad_handler"
      )
    endif()
  endif()
endif()

# Force rebuild of application_build_timestamp.cpp on every build.
add_custom_target(scrite_touch_build_timestamp ALL
  COMMAND "${CMAKE_COMMAND}" -E touch
          "${SCRITE_DESKTOP_SRC_DIR}/core/application_build_timestamp.cpp"
)
add_dependencies(Scrite scrite_touch_build_timestamp)

