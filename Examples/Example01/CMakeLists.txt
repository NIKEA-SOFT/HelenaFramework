#|--------------------------------
#| HF Example01 Project
#|--------------------------------
cmake_minimum_required(VERSION 3.14)

set(HELENA_APP Example01)

project(${HELENA_APP})

file(GLOB_RECURSE HELENA_APP_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

file(GLOB_RECURSE HELENA_APP_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.ipp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

file(GLOB_RECURSE HELENA_APP_SYSTEMS
    "${CMAKE_CURRENT_SOURCE_DIR}/Systems/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Systems/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Systems/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/Systems/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Systems/*.ipp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Systems/*.h"
)

add_executable(${HELENA_APP} ${HELENA_APP_SOURCE} ${HELENA_APP_HEADERS})

source_group("Source" FILES ${HELENA_APP_SOURCE})
source_group("Headers" FILES ${HELENA_APP_HEADERS})
source_group("Systems" FILES ${HELENA_APP_SYSTEMS})

if(WIN32)
    set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /SUBSYSTEM:WINDOWS /PDBSTRIPPED:${HELENA_APP}.pdb")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor")  # Use /Zc:preprocessor for support VA_OPT in MSVC
    endif()
endif()

target_link_libraries(${HELENA_APP} Helena::Helena)
