cmake_policy(SET CMP0076 NEW)

function(Random_add_sources target)
    
    target_sources("${target}" PUBLIC
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/Random_Generator.h
            ${CMAKE_SOURCE_DIR}/src/Random/Random_Generator.cpp
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/Random_MT19937.h
            ${CMAKE_SOURCE_DIR}/src/Random/Random_MT19937.cpp
    )

endfunction()