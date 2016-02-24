message(STATUS "    libusb" )


#rt - clock_gettime
# may not be needed anymore, as of glibc 2.17

# include(CheckLibraryExists)
# CHECK_LIBRARY_EXISTS(rt clock_gettime "time.h" HAVE_RT)

# if( NOT HAVE_RT )
#   message( "--cannot find rt library" )
# endif()

# target_link_libraries( hid-usb -lrt)

#libusb 1.0

find_package( libusb-1.0 )

## NOT TESTED:
IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  find_package( IConv )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

#TODO: ADD ICONV
# set( hidapi_INCLUDE_DIRS ${LIBUSB_1_INCLUDE_DIRS} ${hidapi_SOURCE_DIR}/hidapi/ ${PTHREADS_INCLUDE_DIR})
# set( hidapi_LIBS ${LIBUSB_1_LIBRARIES} ${PTHREADS_LIBRARIES})
# set( hidapi_SRCS hid.c )

include_directories( ${LIBUSB_1_INCLUDE_DIRS} ${PTHREADS_INCLUDE_DIR} ${hidapi_SOURCE_DIR}/hidapi/ )
add_library( hidapi STATIC hid.c )
target_link_libraries( hidapi ${LIBUSB_1_LIBRARIES} ${PTHREADS_LIBRARIES} )
