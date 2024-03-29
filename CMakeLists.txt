cmake_minimum_required(VERSION 3.8)
project(NIH
  LANGUAGES CXX
  VERSION 0.0.0)

list(APPEND CMAKE_MODULE_PATH "${NIH_SOURCE_DIR}/cmake/modules")

option(NIH_ENABLE_TESTS "Enable GTest" ON)
option(NIH_ENABLE_SANITIZERS "Enable sanitizers" OFF)
set(ENABLED_SANITIZERS "address" CACHE STRING
  "Semicolon separated list of sanitizer names. E.g 'address;leak'. Supported sanitizers are
address, leak and thread.")
option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
option(NIH_COMPILE_COLORED_OUTPUT "Enable colored compiler output." ON)

if (NIH_COMPILE_COLORED_OUTPUT AND (CMAKE_GENERATOR STREQUAL "Ninja") AND
    ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
      (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
endif()

if (ENABLE_SANITIZERS)
  include(cmake/sanitizer.cmake)
  enable_sanitizers("${ENABLED_SANITIZERS}")
endif (ENABLE_SANITIZERS)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL "In-source builds not supported.")
endif()

file(GLOB_RECURSE NIH_SOURCES "src/*.cc")

add_library(nih SHARED ${NIH_SOURCES})
add_library(nih::nih ALIAS nih)
set_target_properties(
  nih PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON)
target_include_directories(nih
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>
  PUBLIC
  $<INSTALL_INTERFACE:include/>)

include(GNUInstallDirs)
file(GLOB_RECURSE NIH_INSTALL_HEADERS "include/nih/*.hh")
file(GLOB_RECURSE NIH_INSTALL_HEADERS_H "include/nih/*.h")
install(FILES
  ${NIH_INSTALL_HEADERS}
  DESTINATION
  include/nih/)
install(FILES
  ${NIH_INSTALL_HEADERS_H}
  DESTINATION
  include/nih/)


install(EXPORT nih-targets
  FILE nih-targets.cmake
  NAMESPACE nih::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nih)
install(TARGETS nih
  EXPORT nih-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${LIBLEGACY_INCLUDE_DIRS})

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/nih-config.cmake.in
  ${CMAKE_BINARY_DIR}/cmake/nih-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nih)

write_basic_package_version_file(
  ${CMAKE_BINARY_DIR}/cmake/nih-config-version.cmake
  VERSION ${NIH_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(
  FILES
  ${CMAKE_BINARY_DIR}/cmake/nih-config.cmake
  ${CMAKE_BINARY_DIR}/cmake/nih-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nih)

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable")
if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
endif()

if (NIH_ENABLE_TESTS)
  find_package(GTest REQUIRED)
  file(GLOB_RECURSE TEST_SOURCES "tests/*.cc")
  file(GLOB_RECURSE UNITTEST_SOURCES "tests/unittests/*.cc")

  add_executable(test-nih ${TEST_SOURCES} ${UNITTEST_SOURCES})
  target_include_directories(test-nih
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include"
    PRIVATE "${GTEST_INCLUDE_DIRS}"
    "${CMAKE_CURRENT_LIST_DIR}/include")
  target_link_libraries(test-nih
    GTest::GTest GTest::Main
    nih::nih)
  set_target_properties(
    test-nih PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON)
  if (ENABLE_CLANG_TIDY)
    set_target_properties(
      test-nih PROPERTIES
      CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  endif (ENABLE_CLANG_TIDY)

  enable_testing()
  add_test(TestNIH test-nih)
endif (NIH_ENABLE_TESTS)