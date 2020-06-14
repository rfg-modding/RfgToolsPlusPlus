
cmake_minimum_required (VERSION 3.8)
project (Common VERSION 1.0.0
                  DESCRIPTION "Common"
                  LANGUAGES CXX)

# Get list of source files in project src folders recursively
# Places in SOURCES variable
file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp
)

# Add source to this project's static library (.lib)
add_library(Common STATIC ${SOURCES})

# Add include directories
target_include_directories(Common SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/
)