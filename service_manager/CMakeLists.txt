project(service_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
    src/main.cpp
)

add_executable(service_manager ${SOURCE_FILES})

target_link_libraries(service_manager PRIVATE common Threads::Threads)
