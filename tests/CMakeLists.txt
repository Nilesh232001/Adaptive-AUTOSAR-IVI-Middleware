project(TestSuite)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for the common library
include_directories(${CMAKE_SOURCE_DIR}/common/include)

# Add the serialization tests executable
add_executable(serialization_tests serialization_tests.cpp)

# Link against the common library
target_link_libraries(serialization_tests PRIVATE common)