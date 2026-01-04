project(navigation_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(navigation_service src/main.cpp)

target_link_libraries(navigation_service PRIVATE common Threads::Threads)