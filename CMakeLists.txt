cmake_minimum_required(VERSION 3.24)
project(McpingerCpp)

set(CMAKE_CXX_STANDARD 23)

add_executable(McpingerCpp src/main.cpp
        src/Mcping.cpp
        src/Mcping.h
        src/DataTypesUtils.cpp
        src/DataTypesUtils.h
        src/DataTypesUtils.cpp
)

