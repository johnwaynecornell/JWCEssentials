cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_CXX_STANDARD 17)

project(ProcessHandler)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_library(JWCEssentials SHARED
            JWCEssentials.cpp
            JWCEssentials.h
            JWCEssentials.cpp
            Intern.h
            Intern.h
            utf8_string_handle.cpp
            utf8_string_handle.h
            Paths.cpp
            Paths.h
            Windows/Paths_Windows.cpp
            Escapes.cpp
            Escapes.h
    )

    add_definitions(-D_EXPORT_=__declspec\(dllexport\))
else()
    add_library(JWCEssentials SHARED
            JWCEssentials.cpp
            JWCEssentials.h
            JWCEssentials.cpp
            Intern.h
            Intern.h
            utf8_string_handle.cpp
            utf8_string_handle.h
            Paths.cpp
            Paths.h
            Linux/Paths_Linux.cpp
            Escapes.cpp
            Escapes.h
            Test/test_bash_invoke.cpp
    )

    add_definitions(-D_EXPORT_=)
endif()

#get_target_property(OUTPUT_DIR JWCEssentials OUTPUT_DIRECTORY)
#link_directories( ${OUTPUT_DIR})

add_subdirectory(JWCProcessHandler)

add_executable(test Test/test.cpp Test/test.h)
target_link_libraries(test PUBLIC JWCEssentials)

add_executable(test_bash_invoke Test/test_bash_invoke.cpp)
target_link_libraries(test_bash_invoke PUBLIC JWCEssentials JWCProcessHandler)
