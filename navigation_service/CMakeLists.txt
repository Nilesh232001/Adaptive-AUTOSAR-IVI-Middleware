project(navigation_service)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/common/include)

add_executable(navigation_service src/main.cpp)

target_link_libraries(navigation_service PRIVATE common)