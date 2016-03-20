message(STATUS "    linux hidraw" )

#udev
find_package(UDev)

# include_directories(${UDEV_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/hidapi/)
# link_libraries(${UDEV_LIBRARIES})

# message( "hidapi source dir is: ${hidapi_SOURCE_DIR}" )

# set( hidapi_INCLUDE_DIRS ${UDEV_INCLUDE_DIR} ${hidapi_SOURCE_DIR}/hidapi/)
# set( hidapi_LIBS ${UDEV_LIBRARIES})
# set( hidapi_SRCS hid.c )

# message( "hidapi include dirs are: ${hidapi_INCLUDE_DIRS}" )

include_directories( ${UDEV_INCLUDE_DIR} ${hidapi_SOURCE_DIR}/hidapi/ )
add_library( hidapi STATIC hid.c )
target_link_libraries( hidapi ${UDEV_LIBRARIES} )
# link_directories( hidapi ${UDEV_LIBRARIES} )
