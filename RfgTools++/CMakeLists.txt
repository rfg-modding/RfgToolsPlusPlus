
cmake_minimum_required (VERSION 3.8)
project (RfgTools++ VERSION 1.0.0
                  DESCRIPTION "RfgTools++"
                  LANGUAGES CXX)

# Get list of source files in project src folders recursively
# Places in SOURCES variable
file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/RfgTools++/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RfgTools++/*.cpp
    # TODO: Hook this up as a static lib once it's got a CMakeLists.txt. For now this works
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/BinaryTools/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/BinaryTools/*.cpp

    # Manually add zlib files since we're using a super old version (1.2.3) to attempt RFG compat
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/zlib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/zlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/adler32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/compress.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/zutil.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/inflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/inflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/uncompr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/deflate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/deflate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/crc32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/infback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/inffast.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/inffast.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/inffixed.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/inftrees.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/inftrees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/trees.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/trees.h

    # Add tinyxml2 files
    ${CMAKE_SOURCE_DIR}/dependencies/tinyxml2/tinyxml2.cpp
)

# Add source to this project's static library (.lib)
add_library(RfgTools++ STATIC ${SOURCES})

# Add include directories
target_include_directories(RfgTools++ SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common/
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/RfgTools++/
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/zlib/
    ${CMAKE_SOURCE_DIR}/Dependencies/tinyxml2
    ${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/BinaryTools/ # TODO: Hook this up as a static lib once it's got a CMakeLists.txt
)

#target_link_libraries(RfgTools++ PRIVATE zlibstatic)
target_link_libraries(RfgTools++ PRIVATE Common)