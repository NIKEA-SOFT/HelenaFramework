#|--------------------------------
#| HF Plugin Project
#|--------------------------------
cmake_minimum_required(VERSION 3.14)

set(HELENA_APP HelenaPlugin)

project(${HELENA_APP})

file(GLOB_RECURSE HELENA_APP_SOURCE *.cpp *.cc *.c)
file(GLOB_RECURSE HELENA_APP_HEADERS *.h *.hpp *.ipp)

add_library(${HELENA_APP} SHARED ${HELENA_APP_SOURCE} ${HELENA_APP_HEADERS})

source_group("Source" FILES ${HELENA_APP_SOURCE})
source_group("Headers" FILES ${HELENA_APP_HEADERS})

if(WIN32)
    set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_APP}.pdb")
elseif(UNIX)
    SET_TARGET_PROPERTIES(${HELENA_APP} PROPERTIES PREFIX "")
endif()
target_link_libraries(${HELENA_APP} Helena::Helena)
