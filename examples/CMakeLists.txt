cmake_minimum_required(VERSION 3.8)

find_package(nih)
add_executable(strings-example ${CMAKE_SOURCE_DIR}/strings.cc)
add_executable(json-experimental ${CMAKE_SOURCE_DIR}/json_experimental.cc)
target_link_libraries(json-experimental nih::nih)
target_link_libraries(strings-example nih::nih)