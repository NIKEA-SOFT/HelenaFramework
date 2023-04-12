#|--------------------------------
#| HF Application Project
#|--------------------------------
cmake_minimum_required(VERSION 3.14)

set(HELENA_APP HelenaMain)
project(${HELENA_APP})

HELENA_SOURCE_PRETTY()
HELENA_SOURCE_GLOB(HELENA_APP_SOURCE "${CMAKE_CURRENT_LIST_DIR}" cpp cc c h hpp ipp)
HELENA_SOURCE_GROUP("${CMAKE_CURRENT_LIST_DIR}" "${HELENA_APP_SOURCE}")

add_executable(${HELENA_APP} ${HELENA_APP_SOURCE})
target_link_libraries(${HELENA_APP} PRIVATE Helena::Helena)

if(WIN32)
	# Ignore this is the flag I use them to get .pdb files
	# I use it to analyze code performance when i reverse engineer a binary file
	if(NOT MINGW)
		set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_APP}.pdb")
	endif()
endif()