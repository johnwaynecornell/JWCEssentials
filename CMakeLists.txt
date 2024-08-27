cmake_minimum_required(VERSION 3.28)

#set(CMAKE_C_COMPILER "c")
#set(CMAKE_CXX_COMPILER "c++")

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
# set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/include/JWCEssentials/" ${CMAKE_MODULE_PATH})


add_definitions(-DBUILD_JWCESSENTIALS)

# message(STATUS "${CMAKE_MODULE_PATH}")

project(JWCEssentials)

include( "${CMAKE_SOURCE_DIR}/include/JWCEssentials/JWCEssentials.cmake")

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_library(JWCEssentials SHARED
            include/JWCEssentials/JWCEssentials.h
            include/JWCEssentials/utf8_string_struct.h
            include/JWCEssentials/struct_array_struct.h
            include/JWCEssentials/Escapes.h
            include/JWCEssentials/Paths.h
            include/JWCEssentials/BufferHelper.h

            src/JWCEssentials.cpp
            src/utf8_string_struct.cpp
            src/struct_array_struct.cpp
            src/Paths.cpp
            src/Escapes.cpp
            src/feffect.cpp
            src/BufferHelper.cpp


            src/Platform_Windows/Paths_Windows.cpp
            include/JWCEssentials/feffect.h
    )

elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    add_library(JWCEssentials SHARED
            include/JWCEssentials/JWCEssentials.h
            include/JWCEssentials/utf8_string_struct.h
            include/JWCEssentials/struct_array_struct.h
            include/JWCEssentials/Escapes.h
            include/JWCEssentials/Paths.h
            include/JWCEssentials/BufferHelper.h

            src/JWCEssentials.cpp
            src/utf8_string_struct.cpp
            src/struct_array_struct.cpp
            src/Paths.cpp
            src/Escapes.cpp
            src/feffect.cpp
            src/BufferHelper.cpp

            src/Platform_Linux/Paths_Linux.cpp

            include/JWCEssentials/feffect.h
    )
endif()

target_compile_options(JWCEssentials PUBLIC "-Wno-return-type-c-linkage")
include( "${CMAKE_SOURCE_DIR}/src/Random/Random.cmake")

include( "${CMAKE_SOURCE_DIR}/src/Hasher/Hasher.cmake")

Hasheradd_sources(JWCEssentials)

Random_add_sources(JWCEssentials)

add_subdirectory(src/FromSMHasher)

#get_target_property(OUTPUT_DIR JWCEssentials OUTPUT_DIRECTORY)
#link_directories( ${OUTPUT_DIR})

add_executable(test src/Test/test.cpp src/Test/test.h)
target_link_libraries(test PUBLIC JWCEssentials)

add_executable(feffect src/Utils/feffect.cpp)
add_executable(rand_identifier src/Utils/rand_identifier.cpp)
add_executable(split_arg src/Utils/split_arg.cpp)

target_link_libraries(feffect PUBLIC JWCEssentials)
target_link_libraries(rand_identifier PUBLIC JWCEssentials)
target_link_libraries(split_arg PUBLIC JWCEssentials)

target_link_libraries(JWCEssentials PUBLIC FromSMHasher)

get_target_property(name JWCEssentials OUTPUT_NAME)

if (name STREQUAL "name-NOTFOUND")
    set(name "JWCEssentials")
endif()

isDebug(CrystalCatalystLibrary ISDEBUG)

if (ISDEBUG)
    set(PTH "Debug")
else()
    set(PTH "Release")
endif()

set(PTH "${PTH}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}")

#add_executable(feffect src/Utils/feffect.cpp)
#add_executable(rand_identifier src/Utils/rand_identifier.cpp)
#add_executable(split_arg src/Utils/split_arg.cpp)

add_custom_command(TARGET split_arg POST_BUILD
        COMMAND bash verbose.sh shuttle_to.sh bin "${CMAKE_CURRENT_BINARY_DIR}" "split_arg" "${PTH}" "${CMAKE_CURRENT_BINARY_DIR}/split_arg"
        COMMAND bash verbose.sh shuttle_to.sh lib "${CMAKE_CURRENT_BINARY_DIR}" "JWCEssentials" "${PTH}" "${CMAKE_CURRENT_BINARY_DIR}/split_arg"
        
        COMMAND bash verbose.sh shuttle_to.sh bin "${CMAKE_CURRENT_BINARY_DIR}" "feffect" "${PTH}"
        COMMAND bash verbose.sh shuttle_to.sh bin "${CMAKE_CURRENT_BINARY_DIR}" "rand_identifier" "${PTH}"
)
