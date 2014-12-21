cmake_minimum_required(VERSION 3.0)

project (BWTIL)

include_directories(${PROJECT_SOURCE_DIR}/extern/bitvector/include)

message("Building in ${CMAKE_BUILD_TYPE} mode")

set(CMAKE_CXX_FLAGS "--std=c++11")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -g")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -fstrict-aliasing -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Ofast -fstrict-aliasing -march=native")

add_executable(cw-bwt tools/cw-bwt/cw-bwt.cpp)
add_executable(sFM-index tools/sFM-index/sFM-index.cpp)
add_executable(dB-hash tools/dB-hash/dB-hash.cpp)
add_executable(bwt-check tools/bwt-check/bwt-check.cpp)
add_executable(bwt-to-sa tools/bwt-to-sa/bwt-to-sa.cpp)
add_executable(sa-to-bwt tools/sa-to-bwt/sa-to-bwt.cpp)
add_executable(bwt-invert tools/bwt-invert/bwt-invert.cpp)
add_executable(bwtest tools/test/test.cpp)
