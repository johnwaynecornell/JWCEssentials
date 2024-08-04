cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
# set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/include/JWCEssentials/" ${CMAKE_MODULE_PATH})

add_definitions(-DBUILD_JWCESSENTIALS)

# message(STATUS "${CMAKE_MODULE_PATH}")

project(CommandSpawn)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_library(JWCEssentials SHARED
            include/JWCEssentials/JWCEssentials.h
            include/JWCEssentials/Intern.h
            include/JWCEssentials/utf8_string_struct.h
            include/JWCEssentials/struct_array_struct.h
            include/JWCEssentials/Escapes.h
            include/JWCEssentials/Paths.h

            src/JWCEssentials.cpp
            src/utf8_string_struct.cpp
            src/struct_array_struct.cpp
            src/Paths.cpp
            src/Escapes.cpp
            src/feffect.cpp

            src/Platform_Windows/Paths_Windows.cpp
            include/JWCEssentials/colors.h
    )

elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    add_library(JWCEssentials SHARED
            include/JWCEssentials/JWCEssentials.h
            include/JWCEssentials/Intern.h
            include/JWCEssentials/utf8_string_struct.h
            include/JWCEssentials/struct_array_struct.h
            include/JWCEssentials/Escapes.h
            include/JWCEssentials/Paths.h

            src/JWCEssentials.cpp
            src/utf8_string_struct.cpp
            src/struct_array_struct.cpp
            src/Paths.cpp
            src/Escapes.cpp
            src/feffect.cpp

            src/Platform_Linux/Paths_Linux.cpp
            include/JWCEssentials/colors.h
            include/JWCEssentials/feffect.h
    )
endif()

include( "${CMAKE_SOURCE_DIR}/include/JWCEssentials/JWCEssentials.cmake")

#get_target_property(OUTPUT_DIR JWCEssentials OUTPUT_DIRECTORY)
#link_directories( ${OUTPUT_DIR})

add_subdirectory(Project/JWCCommandSpawn)

add_executable(test src/Test/test.cpp src/Test/test.h)
target_link_libraries(test PUBLIC JWCEssentials)

add_executable(feffect src/Utils/feffect.cpp)
target_link_libraries(feffect PUBLIC JWCEssentials)
