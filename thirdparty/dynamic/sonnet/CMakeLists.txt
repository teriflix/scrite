set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SONNET_USE_WIDGETS OFF CACHE BOOL "" FORCE)
set(SONNET_USE_QML OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

add_subdirectory(sonnet)

# Export include root for consumers that use direct source includes
# like "sonnet/src/core/speller.h".
set(SCRITE_SONNET_INCLUDE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Scrite Sonnet include root")
set(SCRITE_SONNET_BUILD_INCLUDE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/sonnet" CACHE INTERNAL "Scrite Sonnet build include root")
set(SCRITE_SONNET_BUILD_CORE_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/sonnet/src/core" CACHE INTERNAL "Scrite Sonnet core build include dir")

# Force Sonnet artifacts into the project's common binary folder.
set(_scrite_binary_dir "${CMAKE_SOURCE_DIR}/binary")
set(_sonnet_targets KF6SonnetCore sonnet_nsspellchecker sonnet_ispellchecker sonnet_hunspell)
foreach(_t IN LISTS _sonnet_targets)
  if(TARGET ${_t})
    set_target_properties(${_t} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${_scrite_binary_dir}"
      LIBRARY_OUTPUT_DIRECTORY "${_scrite_binary_dir}"
      ARCHIVE_OUTPUT_DIRECTORY "${_scrite_binary_dir}"
      RUNTIME_OUTPUT_DIRECTORY_DEBUG "${_scrite_binary_dir}"
      RUNTIME_OUTPUT_DIRECTORY_RELEASE "${_scrite_binary_dir}"
      RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_scrite_binary_dir}"
      RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${_scrite_binary_dir}"
      LIBRARY_OUTPUT_DIRECTORY_DEBUG "${_scrite_binary_dir}"
      LIBRARY_OUTPUT_DIRECTORY_RELEASE "${_scrite_binary_dir}"
      LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_scrite_binary_dir}"
      LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL "${_scrite_binary_dir}"
      ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${_scrite_binary_dir}"
      ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${_scrite_binary_dir}"
      ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_scrite_binary_dir}"
      ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL "${_scrite_binary_dir}"
    )
  endif()
endforeach()