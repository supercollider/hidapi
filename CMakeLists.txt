cmake_minimum_required(VERSION 2.8)
if(${CMAKE_MAJOR_VERSION} GREATER 2)
  cmake_policy(SET CMP0048 OLD)
endif(${CMAKE_MAJOR_VERSION} GREATER 2)

project(hidapi)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "7")
set(VERSION_PATCH "1")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")


option(HID_DEBUG_PARSER "verbose parser debugging output" OFF)

# option(LIBUSB "use libusb backend" OFF)
# option(HIDRAW "use hidraw backend (linux/freebsd)" ON)

option(HID_EXAMPLE_TEST "build test example" OFF)
option(HID_EXAMPLE_OSC "build osc example" OFF)

option(HID_INSTALL_HUT "install hid usage tables" ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(HIDAPI "default" CACHE STRING "HID API to use (one of {default,hidraw,libusb})")
elseif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  set(HIDAPI "libusb")
endif()

# add our own cmake-modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake_modules/")

if( HID_DEBUG_PARSER )
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

if(CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  add_definitions( -DLINUX_FREEBSD )
  if(HIDAPI STREQUAL "default")
    set( HIDAPI hidraw )
  endif()

  if(NOT HIDAPI MATCHES "^(libusb|hidraw)$")
        message(WARNING "Unrecognised hid API: ${HIDAPI}")
  endif()

  if (HIDAPI STREQUAL hidraw)
    add_subdirectory(linux)
  #   set( hidapi_source ${hidapi_SOURCE_DIR}/linux )
    link_directories( ${UDEV_LIBRARIES} )
  elseif (HIDAPI STREQUAL libusb)
    add_subdirectory(libusb)
  #   set( hidapi_source ${hidapi_SOURCE_DIR}/libusb )
    link_directories( ${LIBUSB_1_LIBRARIES} ${PTHREADS_LIBRARIES} )
  endif()

endif()


if(WIN32)
  add_definitions( -DWIN32 )
  add_subdirectory(windows)
#   set( hidapi_source ${hidapi_SOURCE_DIR}/windows )
  #todo: add library dependencies
endif()

if(APPLE)
  add_definitions( -DAPPLE )
  add_subdirectory(mac)
#   set( hidapi_source ${hidapi_SOURCE_DIR}/mac )
  #todo: add library dependencies - TEST
  find_library(IOKIT_LIBRARY IOKit )
  find_library(COREFOUNDATION_LIBRARY CoreFoundation )
  mark_as_advanced (COREFOUNDATION_LIBRARY
                     IOKIT_LIBRARY)
  set(EXTRA_LIBS ${COREFOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
  #link_directories("-framework IOKit -framework CoreFoundation")
endif()

add_subdirectory(hidapi_parser)

# message( "main: hidapi source dir: ${hidapi_source}" )

if( HID_EXAMPLE_TEST )
  add_subdirectory(hidtest)
  add_subdirectory(hidparsertest)
  if(APPLE)
    add_subdirectory(hidtestosx)
  endif()
  if(WIN32)
    add_subdirectory(hidtestwindows)
  endif()
endif()

if( HID_EXAMPLE_OSC )
  add_subdirectory(hidapi2osc)
endif()

if( HID_DEBUG_PARSER OR HID_INSTALL_HUT )
  # provisional to avoid having to commit to sc master while the HID submodule
  # is not final/added to master
  # the sc-settings for cmake_install_prefix and scappauxresourcesdir are not
  # visible in this scope
  if(APPLE)
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        SET(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/Install")
    endif()
    # this will break if scappbundlename is reassigned in SC
    # similar problem in editors/sc-ide/CMakeLists.txt 284
    set(scappbundlename "SuperCollider")
    set(scappauxresourcesdir "${scappbundlename}/${scappbundlename}.app/Contents/Resources")
    set(HID_HUT_PATH "${CMAKE_INSTALL_PREFIX}/${scappauxresourcesdir}/HID_Support")
  elseif(WIN32)
    set(auxresourcesdir "SuperCollider" CACHE STRING "Resources directory")
    set(HID_HUT_PATH "${CMAKE_INSTALL_PREFIX}/${auxresourcesdir}/HID_Support")
  else()
    set(auxresourcesdir "share/SuperCollider" CACHE STRING "Resources directory")
    set(HID_HUT_PATH "${CMAKE_INSTALL_PREFIX}/${auxresourcesdir}/HID_Support")
  endif(APPLE)
  install(DIRECTORY hut
  DESTINATION ${HID_HUT_PATH}
  FILES_MATCHING PATTERN "*.yaml"
  )
endif()
