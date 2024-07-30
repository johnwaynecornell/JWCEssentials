cmake_minimum_required(VERSION 3.28)

project(JWCProcessHandler CXX)



set(CMAKE_CXX_STANDARD 17)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Platform_Windows")
    add_library(JWCProcessHandler SHARED
            ProcessHandler.cpp
            ProcessHandler.h
            Intern.h

            Platform_Windows/ProcessHandler_Windows.cpp
    )

    add_definitions(-D_EXPORT_=__declspec\(dllexport\))
else()
    add_library(JWCProcessHandler SHARED
            ProcessHandler.cpp
            ProcessHandler.h
            Intern.h

            Platform_Linux/ProcessHandler_Linux.cpp
            Platform_Linux/Args.cpp
            Platform_Linux/Args.h
    )

    add_definitions(-D_EXPORT_=)
endif()

target_link_libraries(JWCProcessHandler PUBLIC JWCEssentials)

get_target_property(OUTPUT_DIR JWCProcessHandler OUTPUT_DIRECTORY)

add_executable(JWCProcessHandler_test test.cpp test.h)
target_link_libraries(JWCProcessHandler_test PUBLIC JWCEssentials JWCProcessHandler)

add_executable(test_bash_invoke test_bash_invoke.cpp)
target_link_libraries(test_bash_invoke PUBLIC JWCEssentials JWCProcessHandler)
