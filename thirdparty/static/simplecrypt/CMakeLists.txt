find_package(Qt6 6.11 REQUIRED COMPONENTS
  Core
  Core5Compat
)

add_library(simplecrypt STATIC 
  simplecrypt.cpp
  simplecrypt.h
)

# Required so that shared libraries (like LLMTask in extra/) can link this static library
set_target_properties(simplecrypt PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

target_include_directories(simplecrypt
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(simplecrypt
  PUBLIC
    Qt6::Core
    Qt6::Core5Compat
)



