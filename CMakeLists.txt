cmake_minimum_required(VERSION 3.24)
project(McpingerCpp)

set(CMAKE_CXX_STANDARD 23)

# Set Boost to use static libraries
set(Boost_USE_STATIC_LIBS ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system)

# Include Boost headers
include_directories(${Boost_INCLUDE_DIRS})

# Add your source files (e.g., main.cpp)
add_executable(McpingerCpp src/main.cpp
        src/Mcping.cpp
        src/Mcping.h
        src/DataTypesUtils.cpp
        src/DataTypesUtils.h
)

# Link Boost libraries to your project
target_link_libraries(McpingerCpp ${Boost_LIBRARIES})