#|--------------------------------
#| HF Application Project
#|--------------------------------
set(HELENA_APP Helena)
file(GLOB_RECURSE HELENA_APP_SOURCE *.cpp *.cc *.c)
file(GLOB_RECURSE HELENA_APP_HEADERS *.h *.hpp *.ipp)
add_executable(${HELENA_APP} ${HELENA_APP_SOURCE} ${HELENA_APP_HEADERS})
source_group("Source" FILES ${HELENA_APP_SOURCE})
source_group("Headers" FILES ${HELENA_APP_HEADERS})

if(WIN32)
    target_link_libraries(${HELENA_APP})
else(UNIX)
    target_link_libraries(${HELENA_APP} pthread dl --coverage)
endif()

if(WIN32)
set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_APP}.pdb")
endif()
