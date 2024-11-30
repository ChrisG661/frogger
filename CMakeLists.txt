cmake_minimum_required (VERSION 3.22)
project(frogger LANGUAGES CXX VERSION 1.0.0)


add_subdirectory(FTXUI)

add_executable(frogger frogger.cpp)

target_link_libraries(frogger
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

set_target_properties(frogger PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)