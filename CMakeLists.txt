# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.6)

# project name and languages
project(WonFinalPathName C)

# libWonFinalPathName.a
add_library(WonFinalPathName STATIC WonFinalPathName.c)

# WonFinalPathNameTest.exe
add_executable(WonFinalPathNameTest WonFinalPathNameTest.c WonFinalPathName.c)
target_link_libraries(WonFinalPathNameTest PRIVATE shlwapi)

##############################################################################
