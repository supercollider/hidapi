message(STATUS "    hidtestosx" )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${hidapi_SOURCE_DIR}/hidapi/
  ${hidapi_SOURCE_DIR}/hidapi_parser/
)


set(hidtestosx_SRCS
  hidtestosx.cpp
)

add_executable( hidtestosx ${hidtestosx_SRCS} )

target_link_libraries(hidtestosx hidapi hidapi_parser ${EXTRA_LIBS})

install(TARGETS hidtestosx DESTINATION bin)
