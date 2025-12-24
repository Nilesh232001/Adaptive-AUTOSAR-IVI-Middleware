project(media_service)

cmake_minimum_required(VERSION 3.10)

set(SOURCE_FILES
    src/main.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/common/include
)

add_executable(media_service ${SOURCE_FILES})

target_link_libraries(media_service
    PRIVATE
    common
    vsomeip3
)