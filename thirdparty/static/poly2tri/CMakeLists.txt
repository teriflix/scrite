add_library(poly2tri STATIC
  common/shapes.cc
  sweep/advancing_front.cc
  sweep/cdt.cc
  sweep/sweep.cc
  sweep/sweep_context.cc
)

target_include_directories(poly2tri
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

