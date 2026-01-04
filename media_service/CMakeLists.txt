project(media_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
    src/main.cpp
)

add_executable(media_service ${SOURCE_FILES})

target_link_libraries(media_service PRIVATE common Threads::Threads)