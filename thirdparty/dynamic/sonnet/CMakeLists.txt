set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SONNET_USE_WIDGETS OFF CACHE BOOL "" FORCE)
set(SONNET_USE_QML OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

if(DEFINED SCRITE_VENDORED_ECM_CONFIG_DIR AND EXISTS "${SCRITE_VENDORED_ECM_CONFIG_DIR}/ECMConfig.cmake")
  set(ECM_DIR "${SCRITE_VENDORED_ECM_CONFIG_DIR}" CACHE PATH "Scrite vendored ECM config directory" FORCE)
endif()

# Sonnet's ecm_setup_version(PROJECT ...) requires this policy in NEW mode.
if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

add_subdirectory(sonnet)

# On Windows, Sonnet's trigram generation runs `parsetrigrams6.exe` during
# build. Since `parsetrigrams` is created in a subdirectory, use a custom
# target here and wire it as a dependency of KF6SonnetCore.
if(WIN32 AND TARGET parsetrigrams AND TARGET KF6SonnetCore)
  add_custom_target(scrite_sonnet_parsetrigrams_runtime
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:parsetrigrams>
      $<TARGET_FILE_DIR:parsetrigrams>
    DEPENDS parsetrigrams
    COMMAND_EXPAND_LISTS
    VERBATIM
  )
  add_dependencies(KF6SonnetCore scrite_sonnet_parsetrigrams_runtime)
endif()

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