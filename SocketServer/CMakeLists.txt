cmake_minimum_required(VERSION 3.20)
project(SocketServer C)

set(CMAKE_C_STANDARD 11)

find_path(TheHeaderFile socketutil.h
        PATHS /mnt/c/Users/Omar/Desktop/SockChat/SocketUtil)

find_library(TheLibrary
        NAMES SocketUtil
        PATHS /mnt/c/Users/Omar/Desktop/SockChat/SocketUtil/cmake-build-debug-wsl)

add_executable(SocketServer main.c)

include_directories(${TheHeaderFile})
target_link_libraries(${PROJECT_NAME} ${TheLibrary})
