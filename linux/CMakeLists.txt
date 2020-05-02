message(STATUS "    linux hidraw" )

find_package(UDev REQUIRED)
add_library(hidapi STATIC hid.c)
target_link_libraries(hidapi PUBLIC ${UDEV_LIBRARIES})
target_include_directories(hidapi PUBLIC ${UDEV_INCLUDE_DIR} ${hidapi_SOURCE_DIR}/hidapi)
