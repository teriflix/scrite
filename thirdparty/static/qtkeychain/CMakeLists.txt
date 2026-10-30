# Keep this local to QtKeyChain so it doesn't force other libs static.
set(BUILD_SHARED_LIBS OFF)
set(BUILD_WITH_QT6 ON CACHE BOOL "" FORCE)
set(BUILD_TEST_APPLICATION OFF CACHE BOOL "" FORCE)
set(BUILD_TRANSLATIONS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

# On Linux, QtKeyChain talks to the secret service over D-Bus, through libsecret.
if(UNIX AND NOT APPLE)
  find_package(Qt6 6.11 REQUIRED COMPONENTS DBus)
endif()

add_subdirectory(qtkeychain)

# Force QtKeyChain static archive output into the project's common binary folder.
if(TARGET qt6keychain)
  set(_scrite_binary_dir "${CMAKE_SOURCE_DIR}/binary")
  set_target_properties(qt6keychain PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${_scrite_binary_dir}"
  )
endif()
