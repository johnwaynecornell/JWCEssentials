cmake_minimum_required(VERSION 3.28)

project(JWCCommandSpawn CXX)

include_directories(include)

set(CMAKE_CXX_STANDARD 17)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "src/Platform_Windows")
    add_library(JWCCommandSpawn SHARED
            src/CommandSpawn.cpp
            include/JWCCommandSpawn/CommandSpawn.h

            src/Platform_Windows/CommandSpawn_Windows.cpp
    )
else()
    add_library(JWCCommandSpawn SHARED
            src/CommandSpawn.cpp
            include/JWCCommandSpawn/CommandSpawn.h

            src/Platform_Linux/CommandSpawn_Linux.cpp
            src/Platform_Linux/Args.cpp
            src/Platform_Linux/Args.h
    )
endif()

add_definitions(-DBUILD_SPAWNEDPROCESS)

include( "${CMAKE_SOURCE_DIR}/include/JWCEssentials/JWCEssentials.cmake")

target_link_libraries(JWCCommandSpawn PUBLIC JWCEssentials)

get_target_property(OUTPUT_DIR JWCCommandSpawn OUTPUT_DIRECTORY)

add_executable(JWCCommandSpawn_test src/test.cpp src/test.h)
target_link_libraries(JWCCommandSpawn_test PUBLIC JWCEssentials JWCCommandSpawn)

add_executable(test_bash_invoke src/test_bash_invoke.cpp)
target_link_libraries(test_bash_invoke PUBLIC JWCEssentials JWCCommandSpawn)
