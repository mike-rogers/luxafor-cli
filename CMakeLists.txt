cmake_minimum_required(VERSION 3.10)

project(LuxaforCLI VERSION 0.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(HIDAPI REQUIRED)
include_directories(${HIDAPI_INCLUDE_DIRS})
set(LIBS ${LIBS} ${HIDAPI_LIBRARIES})

add_executable(LuxaforCLI main.c)
target_link_libraries(LuxaforCLI ${LIBS})

target_include_directories(LuxaforCLI PUBLIC
  "${PROJECT_BINARY_DIR}"
  )
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "luxafor")
