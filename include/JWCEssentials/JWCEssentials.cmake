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

    set(temp_variable "extern \"C\" __attribute__((visibility(\"default\")))")
    set(temp_variable1 "__attribute__((visibility(\"default\")))")

    add_definitions(-D__EXPORT__=${temp_variable})
    add_definitions(-D__IMPORT__=${temp_variable})

    add_definitions(-D__CLASSEXPORT__=${temp_variable1})
    add_definitions(-D__CLASSIMPORT__=${temp_variable1})

else()
    message(SEND_ERROR "platform ${CMAKE_SYSTEM_NAME} not recognized")
endif()

add_compile_options(-Wno-return-type-c-linkage)
