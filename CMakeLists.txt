cmake_minimum_required(VERSION 3.27)

# Build all macOS targets as universal binaries.
if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "macOS universal architectures" FORCE)
endif()

project(Scrite VERSION 2.0.21 LANGUAGES C CXX)

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

# Sonnet's vendored CMake requires Qt 6.8+ (and uses Qt targets).
find_package(Qt6 6.8 REQUIRED COMPONENTS Core)
qt_standard_project_setup(REQUIRES 6.8)

# Sonnet (and some app sources) include Objective-C++ files on macOS.
# Enable this before adding subdirectories so CMAKE_OBJCXX_* rules exist.
if(APPLE)
  enable_language(OBJCXX)
endif()

add_subdirectory(thirdparty)
add_subdirectory(tools)
add_subdirectory(apps)
