add_library(Lcd2004 STATIC
  src/LiquidCrystal.cpp
)

target_include_directories(Lcd2004
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(Lcd2004
  PUBLIC
  pico_stdlib
)
