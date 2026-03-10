# Keep this local to QuaZip so it doesn't force other libs static.
set(BUILD_SHARED_LIBS OFF)
set(QUAZIP_QT_MAJOR_VERSION 6 CACHE STRING "" FORCE)
set(QUAZIP_INSTALL OFF CACHE BOOL "" FORCE)
set(QUAZIP_ENABLE_TESTS OFF CACHE BOOL "" FORCE)

if(WIN32)
  set(_scrite_quazip_wrapper_dir "${CMAKE_CURRENT_LIST_DIR}")
  set(_scrite_local_vcpkg_toolchain "${_scrite_quazip_wrapper_dir}/.vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(_scrite_quazip_manifest_dir "${_scrite_quazip_wrapper_dir}")
  set(_scrite_vcpkg_active OFF)
  set(_scrite_local_zlib_ready OFF)

  if(DEFINED VCPKG_TARGET_TRIPLET AND NOT VCPKG_TARGET_TRIPLET STREQUAL "")
    set(_scrite_quazip_triplet "${VCPKG_TARGET_TRIPLET}")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_scrite_quazip_triplet "x64-windows")
  else()
    set(_scrite_quazip_triplet "x86-windows")
  endif()

  set(_scrite_local_vcpkg_install_root "${_scrite_quazip_wrapper_dir}/vcpkg_installed")
  set(_scrite_local_vcpkg_triplet_root "${_scrite_local_vcpkg_install_root}/${_scrite_quazip_triplet}")
  set(_scrite_local_zlib_include "${_scrite_local_vcpkg_triplet_root}/include")
  set(_scrite_local_zlib_release "${_scrite_local_vcpkg_triplet_root}/lib/zlib.lib")
  set(_scrite_local_zlib_debug "${_scrite_local_vcpkg_triplet_root}/debug/lib/zlibd.lib")
  set(_scrite_local_bzip2_release "${_scrite_local_vcpkg_triplet_root}/lib/bz2.lib")
  set(_scrite_local_bzip2_debug "${_scrite_local_vcpkg_triplet_root}/debug/lib/bz2d.lib")

  if(DEFINED VCPKG_TARGET_TRIPLET OR DEFINED VCPKG_TOOLCHAIN)
    set(_scrite_vcpkg_active ON)
  elseif(DEFINED CMAKE_TOOLCHAIN_FILE)
    string(FIND "${CMAKE_TOOLCHAIN_FILE}" "vcpkg.cmake" _scrite_vcpkg_toolchain_idx)
    if(NOT _scrite_vcpkg_toolchain_idx EQUAL -1)
      set(_scrite_vcpkg_active ON)
    endif()
  endif()

  if(_scrite_vcpkg_active)
    # Pin the manifest scope to the wrapper so root project stays unchanged.
    set(VCPKG_MANIFEST_DIR "${_scrite_quazip_manifest_dir}" CACHE PATH "" FORCE)
    set(VCPKG_MANIFEST_MODE ON CACHE BOOL "" FORCE)
    set(VCPKG_MANIFEST_INSTALL ON CACHE BOOL "" FORCE)
    message(STATUS "Scrite QuaZip: using active vcpkg toolchain with manifest at ${VCPKG_MANIFEST_DIR}")
  elseif(EXISTS "${_scrite_local_vcpkg_toolchain}")
    # Best-effort local activation when root configure did not provide a vcpkg toolchain.
    set(CMAKE_TOOLCHAIN_FILE "${_scrite_local_vcpkg_toolchain}" CACHE FILEPATH "" FORCE)
    set(VCPKG_MANIFEST_DIR "${_scrite_quazip_manifest_dir}" CACHE PATH "" FORCE)
    set(VCPKG_MANIFEST_MODE ON CACHE BOOL "" FORCE)
    set(VCPKG_MANIFEST_INSTALL ON CACHE BOOL "" FORCE)
    include("${_scrite_local_vcpkg_toolchain}")
    set(_scrite_vcpkg_active ON)
    message(STATUS "Scrite QuaZip: activated local vcpkg toolchain at ${_scrite_local_vcpkg_toolchain}")
  else()
    message(STATUS "Scrite QuaZip: vcpkg is not active; enabling reduced-dependency fallback")
  endif()

  if(NOT _scrite_vcpkg_active)
    if(EXISTS "${_scrite_local_zlib_include}/zlib.h"
        AND (EXISTS "${_scrite_local_zlib_release}" OR EXISTS "${_scrite_local_zlib_debug}"))
      list(PREPEND CMAKE_PREFIX_PATH "${_scrite_local_vcpkg_triplet_root}")
      set(ZLIB_ROOT "${_scrite_local_vcpkg_triplet_root}" CACHE PATH "" FORCE)
      set(ZLIB_INCLUDE_DIR "${_scrite_local_zlib_include}" CACHE PATH "" FORCE)
      if(EXISTS "${_scrite_local_zlib_release}")
        set(ZLIB_LIBRARY_RELEASE "${_scrite_local_zlib_release}" CACHE FILEPATH "" FORCE)
        set(ZLIB_LIBRARY "${_scrite_local_zlib_release}" CACHE FILEPATH "" FORCE)
      elseif(EXISTS "${_scrite_local_zlib_debug}")
        set(ZLIB_LIBRARY "${_scrite_local_zlib_debug}" CACHE FILEPATH "" FORCE)
      endif()
      if(EXISTS "${_scrite_local_zlib_debug}")
        set(ZLIB_LIBRARY_DEBUG "${_scrite_local_zlib_debug}" CACHE FILEPATH "" FORCE)
      endif()

      if(EXISTS "${_scrite_local_bzip2_release}" OR EXISTS "${_scrite_local_bzip2_debug}")
        set(BZIP2_ROOT "${_scrite_local_vcpkg_triplet_root}" CACHE PATH "" FORCE)
      else()
        set(QUAZIP_BZIP2 OFF CACHE BOOL "" FORCE)
      endif()

      set(_scrite_local_zlib_ready ON)
      message(STATUS "Scrite QuaZip: using local preinstalled vcpkg packages from ${_scrite_local_vcpkg_triplet_root}")
    endif()

    if(NOT _scrite_local_zlib_ready)
      message(FATAL_ERROR
        "Scrite QuaZip: ZLIB was not found and no active vcpkg toolchain is set.\n"
        "Run thirdparty/static/quazip/bootstrap-vcpkg.ps1, then re-configure the root project.")
    endif()

    # Keep configure working without vcpkg by dropping optional bzip2.
    if(NOT EXISTS "${_scrite_local_bzip2_release}" AND NOT EXISTS "${_scrite_local_bzip2_debug}")
      set(QUAZIP_BZIP2 OFF CACHE BOOL "" FORCE)
    endif()
    # Keep QuaZip on its default zlib discovery path.
    set(QUAZIP_USE_QT_ZLIB OFF CACHE BOOL "" FORCE)
  endif()
else()
  # Non-Windows builds use upstream QuaZip's default dependency discovery.
  set(QUAZIP_USE_QT_ZLIB OFF CACHE BOOL "" FORCE)
  message(STATUS "Scrite QuaZip: using non-Windows default dependency discovery")
endif()

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
