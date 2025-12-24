project(service_manager)

cmake_minimum_required(VERSION 3.10)

# Set the project name
project(ServiceManager)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/../common/include)

# Source files
set(SOURCE_FILES
    src/main.cpp
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link with common library
target_link_libraries(${PROJECT_NAME} common)