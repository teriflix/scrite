add_library(phtranslator SHARED
  LanguageCodes.h
  PhTranslateLib.cpp
  PhTranslateLib.h
  PhTranslator.cpp
  PhTranslator.h
  stdafx.cpp
  stdafx.h
  targetver.h
)

target_include_directories(phtranslator
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(WIN32)
  target_compile_definitions(phtranslator PRIVATE PHTRANSLATELIB_EXPORTS)
endif()

if(APPLE OR UNIX)
  target_compile_definitions(phtranslator PUBLIC PHTRANSLATE_STATICLIB)
  # Symbols in this legacy library are exported via plain C linkage names.
  # Keep default visibility so they remain linkable from Scrite when global
  # visibility defaults are set to hidden by other CMake modules.
  set_target_properties(phtranslator PROPERTIES
    C_VISIBILITY_PRESET default
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
  )
endif()

