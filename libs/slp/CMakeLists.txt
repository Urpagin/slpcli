add_library(slp STATIC src/slp.cpp src/DataTypesUtils.cpp
        include/slp.h)
target_compile_features(slp PUBLIC cxx_std_20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio
    GIT_TAG asio-1-34-2
)
FetchContent_MakeAvailable(asio)

target_include_directories(slp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${asio_SOURCE_DIR}/asio/include
    PRIVATE
        src  # So slp.cpp can find DataTypesUtils.h
)

target_compile_definitions(slp PRIVATE ASIO_STANDALONE)

find_package(Threads REQUIRED)
target_link_libraries(slp PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(slp PRIVATE ws2_32 wsock32)
endif()