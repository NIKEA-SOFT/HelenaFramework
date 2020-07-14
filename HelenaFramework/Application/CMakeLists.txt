#|--------------------------------
#| HF Application Project
#|--------------------------------
set(HELENA_APP HFApplication)

if(UNIX AND (NOT APPLE))
    set(CMAKE_INSTALL_RPATH ".")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif(UNIX AND (NOT APPLE))

file(GLOB_RECURSE HELENA_APP_SOURCE *.h *.hpp *.cpp *.cc *.c)
add_executable(${HELENA_APP} ${HELENA_APP_SOURCE})

if(WIN32)
    target_link_libraries(${HELENA_APP})
else(UNIX)
    target_link_libraries(${HELENA_APP} pthread dl --coverage)
endif()

message(STATUS "OUTPATH: ${HELENA_PROJECT_OUTDIR}/${HELENA_APP}D")

set_target_properties(${HELENA_APP} PROPERTIES OUTPUT_NAME_DEBUG "${HELENA_APP}D")
set_target_properties(${HELENA_APP} PROPERTIES OUTPUT_NAME_RELEASE "${HELENA_APP}R")
set_target_properties(${HELENA_APP} PROPERTIES FOLDER "Application"
    ARCHIVE_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${HELENA_PROJECT_DIR})