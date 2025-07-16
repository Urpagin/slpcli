cmake_minimum_required(VERSION 3.24)
project(slpcli)

set(CMAKE_CXX_STANDARD 23)

add_executable(slpcli src/main.cpp
        src/slp.cpp
        src/slp.h
        src/DataTypesUtils.cpp
        src/DataTypesUtils.h
)

