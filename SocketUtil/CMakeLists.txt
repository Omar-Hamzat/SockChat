cmake_minimum_required(VERSION 3.20)
project(SocketUtil C)

set(CMAKE_C_STANDARD 11)

add_library(SocketUtil STATIC socketutil.c)
