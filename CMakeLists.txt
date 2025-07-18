cmake_minimum_required(VERSION 3.24)
project(slpcli VERSION 2.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Replace the defines with the major and minor versions from the
# project command. So we can include slpcliConfig.h into the program's
# source.
configure_file(slpcliConfig.h.in slpcliConfig.h)
configure_file(version.txt.in version.txt @ONLY)

add_subdirectory(libs/slp)
add_subdirectory(libs/CLI11)

add_executable(slpcli src/main.cpp)


target_link_libraries(slpcli PRIVATE slp PRIVATE CLI11)
target_include_directories(slpcli PUBLIC "${PROJECT_BINARY_DIR}")
