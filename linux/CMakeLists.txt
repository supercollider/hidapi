message(STATUS "    linux hidraw" )

#udev
find_package(UDev)

include_directories( ${UDEV_INCLUDE_DIR} ${hidapi_SOURCE_DIR}/hidapi/ )
add_library( hidapi STATIC hid.c )
target_link_libraries( hidapi ${UDEV_LIBRARIES} )
# link_directories( hidapi ${UDEV_LIBRARIES} )
