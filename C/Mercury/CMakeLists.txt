cmake_minimum_required(VERSION 3.20)
project(Mercury)

set(CMAKE_CXX_STANDARD 14)

include( "$ENV{NewAge}/include/JWCEssentials/JWCEssentials.cmake")

add_library(Mercury SHARED Mercury.c MercuryCPU.c MercuryStrings.c)
target_compile_definitions(Mercury PRIVATE BUILD_MERCURY)

newage_configure_native_paths(Mercury)

add_executable(test test.cpp)
target_link_libraries(test Mercury)

shuttle(Mercury lib)