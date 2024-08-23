cmake_policy(SET CMP0076 NEW)

function(Hasheradd_sources target)

    target_sources("${target}" PUBLIC
            ${CMAKE_SOURCE_DIR}/src/Hasher/HasherFactory.cpp
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/HasherFactory.h

            ${CMAKE_SOURCE_DIR}/src/Hasher/HasherClass.cpp
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/HasherClass.h

            ${CMAKE_SOURCE_DIR}/src/Hasher/HasherClass32.cpp
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/HasherClass32.h

            ${CMAKE_SOURCE_DIR}/src/Hasher/HasherClass64.cpp
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/HasherClass64.h

            ${CMAKE_SOURCE_DIR}/src/Hasher/Hasher_PRNG32.cpp
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/Hasher_PRNG32.h

            ${CMAKE_SOURCE_DIR}/src/Hasher/Hasher_PRNG64.cpp
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/Hasher_PRNG64.h

            ${CMAKE_SOURCE_DIR}/src/Hasher/Hasher_BufferedForCallback32.cpp
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/Hasher_BufferedForCallback32.h

            ${CMAKE_SOURCE_DIR}/src/Hasher/Hasher_BufferedForCallback64.cpp
            ${CMAKE_SOURCE_DIR}/include/JWCEssentials/Hasher_BufferedForCallback64.h
    )
endfunction()
