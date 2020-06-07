
cmake_minimum_required (VERSION 3.8)
project (RfgTools++ VERSION 1.0.0
                  DESCRIPTION "RfgTools++"
                  LANGUAGES CXX)

# Get list of source files in project src folders recursively
# Places in SOURCES variable
file(GLOB_RECURSE SOURCES 
    ${CMAKE_SOURCE_DIR}/RfgTools++/RfgTools++/*.cpp
)

# Add source to this project's static library (.lib)
add_library(RfgTools++ STATIC ${SOURCES})

# Add include directories
target_include_directories(RfgTools++ SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/Common/
    ${CMAKE_SOURCE_DIR}/Dependencies/BinaryTools/ # TODO: Hook this up as a static lib once it's got a CMakeLists.txt
)

target_link_libraries(RfgTools++ PRIVATE Common)