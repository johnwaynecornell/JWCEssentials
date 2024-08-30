if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    include_directories(include_implementation/Platform_Windows)

    set(temp_variable "extern \"C\" __declspec\(dllexport\)")
    set(temp_variable2 "extern \"C\" __declspec\(dllimport\)")

    #define __EXPORT__ extern "C" __declspec(dllexport)
    #define __IMPORT__ extern "C" __declspec(dllimport)

    #define __CLASSEXPORT__ __declspec(dllexport)
    #define __CLASSIMPORT__ __declspec(dllimport)

    add_definitions(-D__EXPORT__=${temp_variable})
    add_definitions(-D__IMPORT__=${temp_variable2})

    add_definitions(-D__CLASSEXPORT__=__declspec\(dllexport\))
    add_definitions(-D__CLASSIMPORT__=__declspec\(dllimport\))

elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")

    include_directories(include_implementation/Platform_Linux)

    #set(temp_variable "extern \"C\" __attribute__((visibility(\"default\")))")
    #set(temp_variable1 "__attribute__((visibility(\"default\")))")
    set(temp_variable "extern \"C\" ")
    set(temp_variable1 "")

    add_definitions(-D__EXPORT__=${temp_variable})
    add_definitions(-D__IMPORT__=${temp_variable})

    add_definitions(-D__CLASSEXPORT__=${temp_variable1})
    add_definitions(-D__CLASSIMPORT__=${temp_variable1})

else()
    message(SEND_ERROR "platform ${CMAKE_SYSTEM_NAME} not recognized")
endif()

function(isDebug target_name result_var)
    # Check for single-configuration generators
    if(CMAKE_BUILD_TYPE)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(${result_var} TRUE PARENT_SCOPE)
        else()
            set(${result_var} FALSE PARENT_SCOPE)
        endif()
    else()
        # For multi-configuration generators, we need to rely on generator expressions.
        # This creates a property on the target which will contain the value "1" if
        # it's a debug build and "0" otherwise.
        set_property(TARGET ${target_name} PROPERTY DEBUG_BUILD $<CONFIG:Debug>)
        get_target_property(is_debug ${target_name} DEBUG_BUILD)
        if(is_debug)
            set(${result_var} TRUE PARENT_SCOPE)
        else()
            set(${result_var} FALSE PARENT_SCOPE)
        endif()
    endif()
endfunction()

function(shuttle target pool)
    isDebug(${target} ISDEBUG)

    if (ISDEBUG)
        set(PTH "Debug")
    else()
        set(PTH "Release")
    endif()

    set(PTH "${PTH}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}")

    # set(TARGET_OUTPUT_NAME "${target_name}.${TARGET_VERSION}")
    get_target_property(name ${target} OUTPUT_NAME)

    if(${name} STREQUAL "name-NOTFOUND")
        set(name "${target}")
    endif()

    set(COMMAND "verbose.sh shuttle_to.sh \"${pool}\" \"${CMAKE_CURRENT_BINARY_DIR}\" \"${name}\" \"${PTH}\"")
    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND bash "${COMMAND}"
    )

endfunction()

add_compile_options(-Wno-return-type-c-linkage)
include_directories( "$ENV{NewAge}/include")

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    link_directories('$ENV{NewAge}/lib/Debug/Windows/AMD64')
    # set(CMAKE_LIBRARY_PATH '$ENV{LD_LIBRARY_PATH}')
    set(CMAKE_LIBRARY_PATH '$ENV{NewAge}/lib/Debug/Windows/AMD64')
    message(STATUS "on windows")
endif()
