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

function(newage_native_toolchain result_var)
    if (MSVC)
        set(toolchain "msvc")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if (WIN32)
            set(toolchain "clang-cl")
        else()
            set(toolchain "clang")
        endif()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(toolchain "gcc")
    else()
        set(toolchain "${CMAKE_CXX_COMPILER_ID}")
        string(TOLOWER "${toolchain}" toolchain)
    endif()

    set(${result_var} "${toolchain}" PARENT_SCOPE)
endfunction()

function(newage_config_for_target target result_var)
    isDebug(${target} ISDEBUG)

    if (ISDEBUG)
        set(config "Debug")
    else()
        set(config "Release")
    endif()

    set(${result_var} "${config}" PARENT_SCOPE)
endfunction()

function(newage_native_lane_for_target target result_var)
    if(DEFINED ENV{NewAge_Config} AND DEFINED ENV{NewAge_Lane})
        set(lane "$ENV{NewAge_Config}/$ENV{NewAge_Lane}")
    elseif(DEFINED ENV{NewAge_Lane})
        newage_config_for_target(${target} config)
        set(env_lane "$ENV{NewAge_Lane}")
        string(REPLACE "\\" "/" env_lane "${env_lane}")

        if(env_lane MATCHES "^(Debug|Release|debug|release)/")
            set(lane "${env_lane}")
        else()
            set(lane "${config}/${env_lane}")
        endif()
    else()
        newage_config_for_target(${target} config)
        newage_native_toolchain(toolchain)

        set(lane "${config}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/${toolchain}")
    endif()

    string(REPLACE "\\" "/" lane "${lane}")
    set(${result_var} "${lane}" PARENT_SCOPE)
endfunction()

function(newage_native_lib_path_for_target target result_var)
    newage_native_lane_for_target(${target} lane)
    set(${result_var} "$ENV{NewAge}/lib/${lane}" PARENT_SCOPE)
endfunction()

function(newage_native_bin_path_for_target target result_var)
    newage_native_lane_for_target(${target} lane)
    set(${result_var} "$ENV{NewAge}/bin/${lane}" PARENT_SCOPE)
endfunction()

function(newage_configure_native_paths target)
    newage_native_lib_path_for_target(${target} newage_lib_path)

    link_directories("${newage_lib_path}")
    list(APPEND CMAKE_LIBRARY_PATH "${newage_lib_path}")
    set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH}" PARENT_SCOPE)

    message(STATUS "NewAge native lib path: ${newage_lib_path}")
endfunction()

function(shuttle target pool)
    newage_native_lane_for_target(${target} PTH)

    get_target_property(name ${target} OUTPUT_NAME)

    if("${name}" STREQUAL "name-NOTFOUND")
        set(name "${target}")
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND bash verbose.sh shuttle_to.sh
            "${pool}"
            "$<TARGET_FILE_DIR:${target}>"
            "${name}"
            "${PTH}"
            "$ENV{NewAge}/bin/${PTH}/split_arg"
    )
endfunction()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options("-Wno-return-type-c-linkage")
endif()

include_directories( "$ENV{NewAge}/include")

