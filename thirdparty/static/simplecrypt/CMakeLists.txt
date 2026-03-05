find_package(Qt6 6.8 REQUIRED COMPONENTS
  Core
  Core5Compat
)

add_library(simplecrypt STATIC 
  simplecrypt.cpp
  simplecrypt.h
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



