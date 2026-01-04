project(hmi_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hmi_client src/main.cpp)

target_link_libraries(hmi_client PRIVATE common Threads::Threads)