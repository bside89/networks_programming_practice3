cmake_minimum_required(VERSION 3.8)
project(TP3)

set(CMAKE_C_STANDARD 99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(SOURCE_FILES main.c lib/tp3opt.c lib/tp3opt.h lib/common.h lib/debug.h lib/routers.c lib/routers.h lib/packet.c lib/packet.h)
add_executable(routers ${SOURCE_FILES})