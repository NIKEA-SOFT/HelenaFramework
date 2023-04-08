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
	# Ignore this is the flag I use them to get .pdb files
	# I use it to analyze code performance when i reverse engineer a binary file
	if(NOT MINGW)
		set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_APP}.pdb")
	else()
		set_target_properties(${HELENA_APP} PROPERTIES PREFIX "")
	endif()
endif()

target_link_libraries(${HELENA_APP} Helena::Helena)
