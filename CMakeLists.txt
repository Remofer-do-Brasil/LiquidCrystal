add_library(Lcd2004 STATIC
  src/LiquidCrystal.cpp
  src/lib/Print.cpp
)

target_include_directories(Lcd2004
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Lcd2004
  PUBLIC
  pico_base_headers

  PRIVATE
  hardware_gpio_headers
  pico_time_headers
)
