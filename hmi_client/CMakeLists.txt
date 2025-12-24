project(hmi_client)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/common/include)

add_executable(hmi_client src/main.cpp)

target_link_libraries(hmi_client PRIVATE someip common)