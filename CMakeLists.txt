cmake_minimum_required(VERSION 3.7)
project(Shell)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES MYSHELL.c)
add_executable(Shell ${SOURCE_FILES})