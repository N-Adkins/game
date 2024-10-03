bgfx_compile_shaders(
  TYPE VERTEX
  SHADERS vs_basic.sc
  VARYING_DEF varying.def.sc
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/include/generated/shaders
  AS_HEADER
)
bgfx_compile_shaders(
  TYPE FRAGMENT
  SHADERS fs_basic.sc
  VARYING_DEF varying.def.sc
  OUTPUT_DIR ${CMAKE_BINARY_DIR}/include/generated/shaders
  AS_HEADER
)

target_sources(game PRIVATE vs_basic.sc fs_basic.sc)
target_include_directories(game PRIVATE ${CMAKE_BINARY_DIR}/include/generated/shaders)
