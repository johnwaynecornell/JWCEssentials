cmake_minimum_required(VERSION 3.28)

project(spawn_bash_probe CXX)

include_directories(include)

set(CMAKE_CXX_STANDARD 17)

include( "$ENV{NewAge}/include/JWCEssentials/JWCEssentials.cmake")

add_executable(spawn_bash_probe spawn_bash_probe.cpp)
newage_configure_native_paths(spawn_bash_probe)

find_library(LIBJWCEssentials JWCEssentials)
find_library(LIBJWCCommandSpawn JWCCommandSpawn)

target_link_libraries(spawn_bash_probe PUBLIC $(LIBJWCEssentials) $(LIBJWCCommandSpawn))
shuttle(spawn_bash_probe bin)
