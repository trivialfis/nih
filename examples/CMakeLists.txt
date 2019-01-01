cmake_minimum_required(VERSION 3.8)

find_package(nih)
add_executable(strings-example ${CMAKE_SOURCE_DIR}/strings.cc)
target_link_libraries(strings-example nih::nih)