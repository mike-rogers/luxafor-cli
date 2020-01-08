cmake_minimum_required(VERSION 3.10)

# set the project name
project(LuxaforCLI VERSION 0.1)

configure_file(main.h.in main.h)

# add the executable
add_executable(LuxaforCLI main.c)

target_include_directories(LuxaforCLI PUBLIC
  "${PROJECT_BINARY_DIR}"
  )
