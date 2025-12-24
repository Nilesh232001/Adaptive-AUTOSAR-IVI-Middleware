project(media_service)

cmake_minimum_required(VERSION 3.10)

set(SOURCE_FILES
    src/main.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/common/include
)

add_executable(media_service src/main.cpp)

target_link_libraries(media_service 
PRIVATE common 
pthread nlohmann_json::nlohmann_json
)