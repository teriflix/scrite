# Platform-specific source buckets only.

set(SCRITE_DESKTOP_WIN_PLATFORM_SOURCES
  "src/core/platformtransliterator_windows.cpp"
  "src/core/platformtransliterator_windows.h"
)

set(SCRITE_DESKTOP_MAC_PLATFORM_SOURCES
  "src/core/platformtransliterator_macos.h"
  "src/core/platformtransliterator_macos.mm"
)

set(SCRITE_DESKTOP_LINUX_PLATFORM_SOURCES
  "src/core/platformtransliterator_linux.cpp"
  "src/core/platformtransliterator_linux.h"
)

set(SCRITE_DESKTOP_WIN_CRASHPAD_PLATFORM_SOURCES
  "src/crashpad/crashpadmodule_win.cpp"
)

set(SCRITE_DESKTOP_MAC_CRASHPAD_PLATFORM_SOURCES
  "src/crashpad/crashpadmodule_mac.cpp"
)

if(WIN32)
  set(SCRITE_DESKTOP_PLATFORM_SOURCES ${SCRITE_DESKTOP_WIN_PLATFORM_SOURCES})
elseif(APPLE)
  set(SCRITE_DESKTOP_PLATFORM_SOURCES ${SCRITE_DESKTOP_MAC_PLATFORM_SOURCES})
else()
  set(SCRITE_DESKTOP_PLATFORM_SOURCES ${SCRITE_DESKTOP_LINUX_PLATFORM_SOURCES})
endif()
