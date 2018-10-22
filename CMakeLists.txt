cmake_minimum_required(VERSION 3.6)
project(ISA)

find_package(LibXml2 REQUIRED)
include_directories(/usr/include/libxml2)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c argChecker.c argChecker.h feeder.c feeder.h queue.c queue.h connect.c connect.h)
add_executable(ISA ${SOURCE_FILES})
