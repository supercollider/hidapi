message(STATUS "    hidapi2osc" )

include(FindPkgConfig)
# pkg_check_modules
pkg_check_modules(liblo REQUIRED liblo)

include_directories(
  ${CMAKE_BINARY_DIR}
  ${liblo_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/hidapi_parser/
)

link_directories(
   ${liblo_LIBRARY_DIRS}
)


##########################################################################
# hidapi2osc
##########################################################################
project(hidapi2osc)

set(hidapi2osc_SRCS
  hidapi2osc.cpp
)

add_executable( hidapi2osc ${hidapi2osc_SRCS} )

target_link_libraries(hidapi2osc ${liblo_LIBRARIES} ${EXTRA_LIBS} hidapi::hidapi hidapi_parser m)

install(TARGETS hidapi2osc DESTINATION bin)
