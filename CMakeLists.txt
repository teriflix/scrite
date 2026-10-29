cmake_minimum_required(VERSION 3.27)

# Build all macOS targets as universal binaries.
if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "macOS universal architectures" FORCE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0 CACHE STRING "macOS deployment target" FORCE)
endif()

# Qt Creator can inject CMAKE_PROJECT_*_INCLUDE_BEFORE entries that point into a
# per-build-directory helper location. If that helper file gets deleted, project()
# fails before languages are enabled. Ignore only missing paths.
foreach(_scrite_include_before_var CMAKE_PROJECT_INCLUDE_BEFORE CMAKE_PROJECT_Scrite_INCLUDE_BEFORE)
  if(DEFINED ${_scrite_include_before_var} AND NOT EXISTS "${${_scrite_include_before_var}}")
    message(WARNING
      "Ignoring stale ${_scrite_include_before_var}='${${_scrite_include_before_var}}' because the file does not exist.")
    unset(${_scrite_include_before_var})
    unset(${_scrite_include_before_var} CACHE)
  endif()
endforeach()

project(Scrite VERSION 3.3.3 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Match the existing qmake behavior of producing artifacts under repo-root/binary/
set(SCRITE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/binary")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SCRITE_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${SCRITE_OUTPUT_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${SCRITE_OUTPUT_DIR}")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Generate a PDB for Release builds on Windows so crash dumps can be symbolicated.
if(MSVC)
  add_compile_options(/Zi)
  add_link_options(/DEBUG /OPT:REF /OPT:ICF)
endif()

# Sonnet's vendored CMake requires Qt 6.11+ (and uses Qt targets).
find_package(Qt6 6.11 REQUIRED COMPONENTS Core)
qt_standard_project_setup(REQUIRES 6.11)

# Sonnet (and some app sources) include Objective-C++ files on macOS.
# Enable this before adding subdirectories so CMAKE_OBJCXX_* rules exist.
if(APPLE)
  enable_language(OBJCXX)
endif()

# Production-only startup checks: NSIS migration guard, license acceptance, and
# IEDN Technologies org name. Pass -DSCRITE_PRODUCTION_BUILD=ON from the
# production build system (which lives outside this repository). Open-source
# builds leave this OFF and continue to use "Scrite" as the organization name.
# Production builds also embed extensions.qml, so that only the extensions it
# imports can be loaded. Declared here because both extra and apps use it.
option(SCRITE_PRODUCTION_BUILD
       "Enable production-only startup checks and IEDN org name"
       OFF)

add_subdirectory(thirdparty)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extra/CMakeLists.txt")
    set(SCRITE_EXTRA TRUE)
else()
    set(SCRITE_EXTRA FALSE)
endif()

add_subdirectory(apps)

# Extra is added after apps, so that it can refer to the Scrite target.
if(SCRITE_EXTRA)
    add_subdirectory(extra)
endif()
