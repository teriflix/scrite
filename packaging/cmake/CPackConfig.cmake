# Common CPack configuration for Scrite.
# Included from the root CMakeLists.txt (before include(CPack)).

set(CPACK_PACKAGE_NAME "Scrite")
set(CPACK_PACKAGE_VENDOR "VCreate Logic Pvt. Ltd.")
set(CPACK_PACKAGE_CONTACT "support@scrite.io")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.scrite.io")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multilingual Screenplay Writing App")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/packaging/assets/windows/license.txt")

# All packages land in binary/packages/
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_SOURCE_DIR}/binary/packages")

# Suppress CPack from installing CMake install() rules by default;
# each platform module sets CPACK_INSTALLED_DIRECTORIES or install() handles it.
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Scrite")

if(APPLE)
    include("${CMAKE_SOURCE_DIR}/packaging/cmake/CPackMacOS.cmake")
elseif(WIN32)
    include("${CMAKE_SOURCE_DIR}/packaging/cmake/CPackWindows.cmake")
elseif(UNIX)
    include("${CMAKE_SOURCE_DIR}/packaging/cmake/CPackLinux.cmake")
endif()

include(CPack)
