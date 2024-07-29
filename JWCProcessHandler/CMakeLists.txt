cmake_minimum_required(VERSION 3.28)

project(JWCProcessHandler CXX)



set(CMAKE_CXX_STANDARD 17)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_library(JWCProcessHandler SHARED
            ProcessHandler.cpp
            ProcessHandler.h
            Windows/ProcessHandler_Windows.cpp
            ProcessHandler.cpp
            Intern.h
    )

    add_definitions(-D_EXPORT_=__declspec\(dllexport\))
else()
    add_library(JWCProcessHandler SHARED
            ProcessHandler.cpp
            ProcessHandler.h
            Linux/ProcessHandler_Linux.cpp
            ProcessHandler.cpp
            Intern.h
            Intern.h
    )

    add_definitions(-D_EXPORT_=)
endif()

target_link_libraries(JWCProcessHandler PUBLIC JWCEssentials)

#get_target_property(OUTPUT_DIR JWCProcessHandler OUTPUT_DIRECTORY)

#link_directories( ${OUTPUT_DIR})

add_executable(JWCProcessHandler_test test.cpp test.h)
target_link_libraries(JWCProcessHandler_test PUBLIC JWCEssentials JWCProcessHandler)
