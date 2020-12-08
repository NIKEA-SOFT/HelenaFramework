#|--------------------------------
#| HF Application Project
#|--------------------------------
set(HELENA_APP Helena)

file(GLOB_RECURSE HELENA_APP_SOURCE *.h *.hpp *.cpp *.cc *.c)
add_executable(${HELENA_APP} ${HELENA_APP_SOURCE})

if(WIN32)
    target_link_libraries(${HELENA_APP})
else(UNIX)
    target_link_libraries(${HELENA_APP} pthread dl --coverage)
endif()

if(WIN32)
set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_APP}.pdb")
endif()