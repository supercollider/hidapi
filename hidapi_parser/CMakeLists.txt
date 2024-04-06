message(STATUS "    hidapi_parser" )
project(hidapi_parser)
find_package(hidapi REQUIRED)
option(HID_DEBUG_PARSER "verbose parser debugging output" OFF)
option(HID_PARSER_TEST "install hidparsertest" OFF)
if( HID_DEBUG_PARSER )
  add_definitions( -DDEBUG_PARSER )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  add_definitions( -DLINUX_FREEBSD )
endif()

set(SOURCES hidapi_parser.c)
set(HEADERS hidapi_parser.h)
add_library( hidapi_parser STATIC ${SOURCES} )

target_link_libraries(hidapi_parser hidapi::hidapi)
target_include_directories(hidapi_parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
