project(common)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common library source files
set(COMMON_SOURCES
    src/common.cpp
    src/logging.cpp
    src/persistence.cpp
    src/someip.cpp
    src/someip_shim.cpp
)

# Create the common library
add_library(common STATIC ${COMMON_SOURCES})

# Set public include directories
target_include_directories(common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link required libraries (nlohmann_json is header-only, included via -I flags)
target_link_libraries(common PUBLIC Threads::Threads)
