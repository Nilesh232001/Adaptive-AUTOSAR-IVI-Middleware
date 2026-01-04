project(tests LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the serialization tests executable
add_executable(serialization_tests serialization_tests.cpp)

# Link against the common library
target_link_libraries(serialization_tests PRIVATE common Threads::Threads)

# Enable testing
enable_testing()
add_test(NAME SerializationTests COMMAND serialization_tests)