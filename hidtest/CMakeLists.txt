message(STATUS "    hidtest" )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${hidapi_SOURCE_DIR}/hidapi/
  ${hidapi_SOURCE_DIR}/hidapi_parser/
)


set(hidtest_SRCS
  hidtest.cpp
)

add_executable( hidtest ${hidtest_SRCS} )

target_link_libraries(hidtest hidapi hidapi_parser ${EXTRA_LIBS})

install(TARGETS hidtest DESTINATION bin)
