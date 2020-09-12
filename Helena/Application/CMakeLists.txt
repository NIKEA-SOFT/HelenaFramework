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

set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_APP}.pdb")
#set_target_properties(${HELENA_APP} PROPERTIES OUTPUT_NAME_DEBUG "${HELENA_APP}_DBG")
#set_target_properties(${HELENA_APP} PROPERTIES OUTPUT_NAME_RELEASE "${HELENA_APP}_REL")
set_target_properties(${HELENA_APP} PROPERTIES FOLDER "Application"
    ARCHIVE_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR})