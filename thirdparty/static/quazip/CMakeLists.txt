# Keep this local to QuaZip so it doesn't force other libs static.
set(BUILD_SHARED_LIBS OFF)
set(QUAZIP_QT_MAJOR_VERSION 6 CACHE STRING "" FORCE)
set(QUAZIP_INSTALL OFF CACHE BOOL "" FORCE)
set(QUAZIP_ENABLE_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(quazip)

# Force QuaZip static archive output into the project's common binary folder.
if(TARGET QuaZip)
  set(_scrite_binary_dir "${CMAKE_SOURCE_DIR}/binary")
  set_target_properties(QuaZip PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${_scrite_binary_dir}"
  )
endif()
