cmake_minimum_required(VERSION 2.6)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

#set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/.. )

project (BWTIL)

include_directories(${PROJECT_SOURCE_DIR}/extern/bitvector/include)

message("Building in ${CMAKE_BUILD_TYPE} mode")

set(CMAKE_CXX_FLAGS "--std=c++11")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -g")
set(CMAKE_CXX_FLAGS_RELEASE "-g -ggdb -Ofast -fstrict-aliasing -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -ggdb -Ofast -fstrict-aliasing -march=native")

add_executable(cw-bwt tools/cw-bwt/cw-bwt.cpp)
add_executable(sFM-index tools/sFM-index/sFM-index.cpp)
add_executable(dB-hash tools/dB-hash/dB-hash.cpp)
add_executable(bwt-check tools/bwt-check/bwt-check.cpp)
add_executable(bwt-to-sa tools/bwt-to-sa/bwt-to-sa.cpp)
add_executable(sa-to-bwt tools/sa-to-bwt/sa-to-bwt.cpp)
add_executable(bwt-invert tools/bwt-invert/bwt-invert.cpp)
add_executable(bwtest tools/test/test.cpp)

