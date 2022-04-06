#|--------------------------------
#| HF Application Project
#|--------------------------------
cmake_minimum_required(VERSION 3.20)

set(HELENA_APP HelenaPlugin)

project(${HELENA_APP})

file(GLOB_RECURSE HELENA_APP_SOURCE *.cpp *.cc *.c)
file(GLOB_RECURSE HELENA_APP_HEADERS *.h *.hpp *.ipp)

add_library(${HELENA_APP} SHARED ${HELENA_APP_SOURCE} ${HELENA_APP_HEADERS})

source_group("Source" FILES ${HELENA_APP_SOURCE})
source_group("Headers" FILES ${HELENA_APP_HEADERS})

target_compile_features(${HELENA_APP} PRIVATE cxx_std_20)

if(WIN32)
    set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_APP}.pdb")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor")  # Use /Zc:preprocessor for support VA_OPT in MSVC
    endif()

elseif(UNIX)
    SET_TARGET_PROPERTIES(${HELENA_APP} PROPERTIES PREFIX "")
endif()
