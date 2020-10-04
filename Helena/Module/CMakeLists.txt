#|--------------------------------
#| HF Module Builder
#|--------------------------------
option(HELENA_FLAG_PDB     "Build with pdb"    ON)

function(HF_BUILD_MODULE HELENA_MODULE)
    file(GLOB_RECURSE HELENA_MODULE_SOURCE *.h *.hpp *.cpp *.cc *.c)
    add_library(${HELENA_MODULE} SHARED ${HELENA_MODULE_SOURCE})

    if(WIN32)
        add_definitions(-D_USRDLL)
        target_link_libraries(${HELENA_MODULE})
    else(UNIX)
        target_link_libraries(${HELENA_MODULE} pthread dl --coverage)
    endif()

    include_directories(${HELENA_PROJECT_DIR}/Helena/Module/${HELENA_MODULE})
    
    set_target_properties(${HELENA_MODULE} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_MODULE}.pdb")
    #set_target_properties(${HELENA_MODULE} PROPERTIES OUTPUT_NAME_DEBUG "${HELENA_MODULE}_DBG")
    #set_target_properties(${HELENA_MODULE} PROPERTIES OUTPUT_NAME_RELEASE "${HELENA_MODULE}_REL")
    set_target_properties(${HELENA_MODULE} PROPERTIES FOLDER "Module"
        ARCHIVE_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR})

endfunction(HF_BUILD_MODULE)

#|--------------------------------
#| HF Modules
#|--------------------------------
add_subdirectory(ModuleTestA)
add_subdirectory(ModuleTestB)
add_subdirectory(ModuleLog)
add_subdirectory(ModuleConfig)