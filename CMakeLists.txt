cmake_minimum_required(VERSION 2.8)

project(hidapi)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "7")
set(VERSION_PATCH "1")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")


option(DEBUG_PARSER "verbose parser debugging output" OFF)

option(LIBUSB "use libusb backend" OFF)
option(HIDRAW "use hidraw backend (linux/freebsd)" ON)

option(EXAMPLE_TEST "build test example" ON)
option(EXAMPLE_OSC "build osc example" ON)


# add our own cmake-modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake_modules/")

if( DEBUG_PARSER )
  add_definitions( -DDEBUG_PARSER )
endif()

# some default libraries
if (NOT WIN32)
	find_package(Pthreads)
	if (NOT PTHREADS_FOUND)
	    message(SEND_ERROR "cannot find libpthreads")
	endif()
endif()


# directories
add_subdirectory(hidapi)

if(LIBUSB)
  add_subdirectory(libusb)
#   set( hidapi_source ${hidapi_SOURCE_DIR}/libusb )
  link_directories( ${LIBUSB_1_LIBRARIES} ${PTHREADS_LIBRARIES} )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND HIDRAW)
  add_subdirectory(linux)
#   set( hidapi_source ${hidapi_SOURCE_DIR}/linux )
  link_directories( ${UDEV_LIBRARIES} )
endif()

if(WIN32)
  add_subdirectory(windows)
#   set( hidapi_source ${hidapi_SOURCE_DIR}/windows )
  #todo: add library dependencies
endif()
if(APPLE)
  add_subdirectory(mac)
#   set( hidapi_source ${hidapi_SOURCE_DIR}/mac )
  #todo: add library dependencies - TEST
  link_directories("-framework IOKit -framework CoreFoundation")
endif()

add_subdirectory(hidapi_parser)

# message( "main: hidapi source dir: ${hidapi_source}" )

if( EXAMPLE_TEST )
  add_subdirectory(hidtest)
  add_subdirectory(hidparsertest)
endif()

if( EXAMPLE_OSC )
  add_subdirectory(hidapi2osc)
endif()