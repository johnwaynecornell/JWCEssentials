cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_CXX_STANDARD 17)

project(ProcessHandler)

include_directories(include)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_library(JWCEssentials SHARED
            include/JWCEssentials/JWCEssentials.h
            include/JWCEssentials/Intern.h
            include/JWCEssentials/utf8_string_handle.h
            include/JWCEssentials/Escapes.h

            src/JWCEssentials.cpp
            src/utf8_string_handle.cpp
            src/Paths.cpp
            include/JWCEssentials/Paths.h
            src/Escapes.cpp

            src/Platform_Windows/Paths_Windows.cpp
    )

    add_definitions(-D_EXPORT_=__declspec\(dllexport\))
else()
    add_library(JWCEssentials SHARED
            include/JWCEssentials/JWCEssentials.h
            include/JWCEssentials/Intern.h
            include/JWCEssentials/utf8_string_handle.h
            include/JWCEssentials/Escapes.h

            src/JWCEssentials.cpp
            src/utf8_string_handle.cpp
            src/Paths.cpp
            include/JWCEssentials/Paths.h
            src/Escapes.cpp

            src/Platform_Linux/Paths_Linux.cpp
    )

    add_definitions(-D_EXPORT_=)
endif()

#get_target_property(OUTPUT_DIR JWCEssentials OUTPUT_DIRECTORY)
#link_directories( ${OUTPUT_DIR})

add_subdirectory(Project/JWCProcessHandler)

add_executable(test src/Test/test.cpp src/Test/test.h)
target_link_libraries(test PUBLIC JWCEssentials)
