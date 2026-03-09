if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ecm/modules/ECMSetupVersion.cmake")
  # Expose vendored ECM root for downstream thirdparty consumers (Sonnet).
  set(SCRITE_VENDORED_ECM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ecm" CACHE PATH "Vendored ECM root path" FORCE)
  set(SCRITE_VENDORED_ECM_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ecm-cmake" CACHE PATH "Vendored ECM config shim path" FORCE)
endif()
