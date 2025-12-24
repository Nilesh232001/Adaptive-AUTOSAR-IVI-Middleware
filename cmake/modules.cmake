set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for common utilities
include_directories(${CMAKE_SOURCE_DIR}/common/include)

# Add subdirectories for each service
add_subdirectory(service_manager)
add_subdirectory(media_service)
add_subdirectory(navigation_service)
add_subdirectory(climate_service)
add_subdirectory(hmi_client)

# Enable testing
enable_testing()
add_subdirectory(tests)