message(STATUS "    Windows" )

include_directories( ${hidapi_SOURCE_DIR}/hidapi/ )
add_library( hidapi STATIC hid.c )
target_link_libraries( hidapi setupapi )
