
set(BUILD_RFGTOOLS_TEST_APP TRUE)

# Top level project. Treating this like a VS solution
cmake_minimum_required (VERSION 3.8)
project ("RfgTools++ Solution")

option(RFGTOOLS_BUILD_ZLIB "If disabled RfgTools++ won't build zlib and instead will rely on another module building and exporting it's target" ON)

set(CMAKE_CXX_STANDARD           20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)
set(CMAKE_CXX_EXTENSIONS         OFF)

add_subdirectory("Common")
if(BUILD_RFGTOOLS_TEST_APP)
    add_subdirectory("Test App")
endif()
add_subdirectory("RfgTools++")