message( "===hidtestwindows cmakelists===" )

include_directories(
  ${CMAKE_BINARY_DIR}
  ${hidapi_SOURCE_DIR}/hidapi/
  ${hidapi_SOURCE_DIR}/hidapi_parser/
)


set(hidtest_SRCS
  hidtestwindows.cpp
)

add_executable( hidtestwindows ${hidtest_SRCS} )

target_link_libraries(hidtestwindows hidapi hidapi_parser ${EXTRA_LIBS})

install(TARGETS hidtestwindows DESTINATION bin)
