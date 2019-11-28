cmake_minimum_required(VERSION 3.15)
project(Socket_Programming_Server)

set(CMAKE_CXX_STANDARD 17)
find_package (Threads REQUIRED)

add_executable(Socket_Programming main.cpp RequestProcessor.cpp RequestProcessor.h StringUtils.cpp StringUtils.h)