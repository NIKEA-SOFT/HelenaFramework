#|--------------------------------
#| HF TestModuleA Project
#|--------------------------------
set(HELENA_MODULE TestModuleA)

file(GLOB_RECURSE HELENA_MODULE_SOURCE *.h *.hpp *.cpp *.cc *.c)
add_library(${HELENA_MODULE} SHARED ${HELENA_MODULE_SOURCE})

if(WIN32)
    target_link_libraries(${HELENA_MODULE})
else(UNIX)
    target_link_libraries(${HELENA_MODULE} pthread dl --coverage)
endif()

set_target_properties(${HELENA_MODULE} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_MODULE}.pdb" )
set_target_properties(${HELENA_MODULE} PROPERTIES FOLDER "Module"
    ARCHIVE_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR})
