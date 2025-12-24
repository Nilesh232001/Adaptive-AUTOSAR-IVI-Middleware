project(climate_service)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/common/include)

add_executable(climate_service src/main.cpp)

target_link_libraries(climate_service PRIVATE common pthread nlohmann_json::nlohmann_json)