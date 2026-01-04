project(climate_service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(climate_service src/main.cpp)

target_link_libraries(climate_service PRIVATE common Threads::Threads nlohmann_json::nlohmann_json)