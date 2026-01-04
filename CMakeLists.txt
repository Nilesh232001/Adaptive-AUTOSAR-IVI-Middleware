cmake_minimum_required(VERSION 3.15)

project(Adaptive_AUTOSAR_IVI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Add local include directories for nlohmann_json (header-only library)
# Prefer the project's `include/` directory so builds work without vcpkg
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find required packages
find_package(nlohmann_json QUIET CONFIG)
if(NOT nlohmann_json_FOUND)
    # nlohmann_json is header-only, try to find it in common include paths
    # or assume it's available as system headers
    message(STATUS "nlohmann_json not found via find_package, using local headers from ${CMAKE_SOURCE_DIR}/include")
    
    # Create an INTERFACE library for the header-only nlohmann_json
    if(NOT TARGET nlohmann_json::nlohmann_json)
        add_library(nlohmann_json INTERFACE)
        add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
        target_include_directories(nlohmann_json INTERFACE ${CMAKE_SOURCE_DIR}/include)
    endif()
endif()
find_package(Threads REQUIRED)

# Add subdirectories for each component
add_subdirectory(common)
add_subdirectory(service_manager)
add_subdirectory(media_service)
add_subdirectory(navigation_service)
add_subdirectory(climate_service)
add_subdirectory(hmi_client)
add_subdirectory(tests)