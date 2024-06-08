cmake_minimum_required(VERSION 3.6)
project(hidapi_tools)
option(HID_EXAMPLE_OSC "build osc example" OFF)
option(HID_INSTALL_HUT "install hid usage tables" ON)
find_package(hidapi REQUIRED)

add_subdirectory(hidapi_parser)


if( HID_PARSER_TEST )
  add_subdirectory(hidparsertest)
endif()

if( HID_EXAMPLE_OSC )
  add_subdirectory(hidapi2osc)
endif()

if( HID_INSTALL_HUT )
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
