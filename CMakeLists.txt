cmake_minimum_required(VERSION 3.18)

project(LuxaforCLI VERSION 1.1.0 LANGUAGES C)

option(LUXAFOR_VENDORED_HIDAPI "Build hidapi from source instead of using an installed copy" OFF)

if(NOT LUXAFOR_VENDORED_HIDAPI)
  find_package(hidapi CONFIG QUIET)
endif()

if(NOT hidapi_FOUND)
  message(STATUS "hidapi not found on system; fetching and building it from source")
  include(FetchContent)
  set(BUILD_SHARED_LIBS OFF)
  set(HIDAPI_WITH_TESTS OFF)
  # hidraw is the right backend for this device; the libusb backend would
  # drag in a libusb-1.0 build dependency on Linux for nothing
  set(HIDAPI_WITH_LIBUSB OFF)
  FetchContent_Declare(
    hidapi
    GIT_REPOSITORY https://github.com/libusb/hidapi.git
    GIT_TAG hidapi-0.15.0
  )
  FetchContent_MakeAvailable(hidapi)
endif()

add_executable(LuxaforCLI main.c)
target_link_libraries(LuxaforCLI PRIVATE hidapi::hidapi)
target_compile_definitions(LuxaforCLI PRIVATE LUXAFOR_VERSION="${PROJECT_VERSION}")

if(UNIX AND NOT APPLE)
  target_link_libraries(LuxaforCLI PRIVATE m) # pow() needs libm on Linux
endif()

# The MSVC C runtime has no getopt, so fetch a BSD-licensed port and compile
# it into the target directly. SOURCE_SUBDIR points at a directory with no
# CMakeLists.txt on purpose: upstream's own CMakeLists only builds its test
# suite and is incompatible with CMake 4.
if(WIN32)
  include(FetchContent)
  FetchContent_Declare(
    getopt_port
    GIT_REPOSITORY https://github.com/kimgr/getopt_port.git
    GIT_TAG 9d3d387087d252970923db7f297f681622c4e026
    SOURCE_SUBDIR no_cmake
  )
  FetchContent_MakeAvailable(getopt_port)
  target_sources(LuxaforCLI PRIVATE ${getopt_port_SOURCE_DIR}/getopt.c)
  target_include_directories(LuxaforCLI PRIVATE ${getopt_port_SOURCE_DIR})
endif()

if(MSVC)
  target_compile_options(LuxaforCLI PRIVATE /W4)
  target_compile_definitions(LuxaforCLI PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(LuxaforCLI PRIVATE -Wall -Wextra)
endif()

set_target_properties(LuxaforCLI PROPERTIES PREFIX "")
set_target_properties(LuxaforCLI PROPERTIES OUTPUT_NAME "luxafor")
