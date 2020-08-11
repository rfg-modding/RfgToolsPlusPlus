
set(BUILD_RFGTOOLS_TEST_APP TRUE)

# Top level project. Treating this like a VS solution
cmake_minimum_required (VERSION 3.8)
project ("RfgTools++ Solution")

# Update git submodules
include("GitSubmodule.cmake")

# Error if submodules not loaded. Just check one of them for now
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/dependencies/zlib/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)
set(CMAKE_CXX_EXTENSIONS         OFF)

# Include sub-projects.
add_subdirectory("Dependencies/zlib")
# Rename this again because zlib infuriatingly renames it and causes compile errors
file(RENAME ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/zlib/zconf.h.included ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/zlib/zconf.h)

add_subdirectory("Common")
if(BUILD_RFGTOOLS_TEST_APP)
    add_subdirectory("Test App")
endif()
add_subdirectory("RfgTools++")