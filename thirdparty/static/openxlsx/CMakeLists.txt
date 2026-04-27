# Keep this local to OpenXLSX so it doesn't force other libs static.
set(BUILD_SHARED_LIBS OFF)
set(OPENXLSX_SHARED_LIBRARY OFF CACHE BOOL "" FORCE)
set(OPENXLSX_ENABLE_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(openxlsx)

# Force OpenXLSX static archive output into the project's common binary folder.
if(TARGET OpenXLSX)
  set(_scrite_binary_dir "${CMAKE_SOURCE_DIR}/binary")
  set_target_properties(OpenXLSX PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_scrite_binary_dir}"
    ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${_scrite_binary_dir}"
  )
endif()
