set(ECM_FOUND TRUE)
set(ECM_VERSION "6.23.0")

if(DEFINED SCRITE_VENDORED_ECM_ROOT AND EXISTS "${SCRITE_VENDORED_ECM_ROOT}/modules/ECMSetupVersion.cmake")
  set(_scrite_ecm_root "${SCRITE_VENDORED_ECM_ROOT}")
else()
  set(_scrite_ecm_root "${CMAKE_SOURCE_DIR}/thirdparty/source/ecm")
endif()

if(NOT EXISTS "${_scrite_ecm_root}/modules/ECMSetupVersion.cmake")
  message(FATAL_ERROR "Vendored ECM modules were not found at: ${_scrite_ecm_root}")
endif()

set(ECM_MODULE_PATH "${_scrite_ecm_root}/modules")
set(ECM_FIND_MODULE_DIR "${_scrite_ecm_root}/find-modules")
set(ECM_KDE_MODULE_DIR "${_scrite_ecm_root}/kde-modules")
set(ECM_MODULE_DIR "${_scrite_ecm_root}/modules")
set(ECM_PREFIX "${_scrite_ecm_root}")
set(ECM_MODULE_PATH "${ECM_MODULE_DIR}" "${ECM_FIND_MODULE_DIR}" "${ECM_KDE_MODULE_DIR}")
set(ECM_GLOBAL_FIND_VERSION "${ECM_FIND_VERSION}")

# Match upstream ECMConfig behavior so downstream include() and helper macros work.
include("${ECM_MODULE_DIR}/ECMUseFindModules.cmake")
