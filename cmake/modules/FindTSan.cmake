set(TSan_LIB_NAME tsan)

find_library(TSan_LIBRARY
  NAMES libtsan.so libtsan.so.0 libtsan.so.0.0.0
  PATHS ${SANITIZER_PATH} ${CMAKE_PREFIX_PATH}/lib ${CMAKE_PREFIX_PATH}/lib64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TSan DEFAULT_MSG
  TSan_LIBRARY)

mark_as_advanced(
  TSan_LIBRARY
  TSan_LIB_NAME)