cmake_minimum_required(VERSION 3.6)
project(ISA)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c argChecker.c argChecker.h feeder.c feeder.h)
add_executable(ISA ${SOURCE_FILES})