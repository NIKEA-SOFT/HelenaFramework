#|--------------------------------
#| HF Application Project
#|--------------------------------
cmake_minimum_required(VERSION 3.14)

set(HELENA_APP HelenaMain)
#set(LIB_LUA_DIR ${HELENA_PROJECT_DIR}/Dependencies/lua)

project(${HELENA_APP})

file(GLOB_RECURSE HELENA_APP_SOURCE *.cpp *.cc *.c)
file(GLOB_RECURSE HELENA_APP_HEADERS *.h *.hpp *.ipp)

#file(GLOB_RECURSE LIB_LUA_SOURCE ${LIB_LUA_DIR}/*.cpp ${LIB_LUA_DIR}/*.cc ${LIB_LUA_DIR}/*.c)
#file(GLOB_RECURSE LIB_LUA_HEADERS ${LIB_LUA_DIR}/*.h ${LIB_LUA_DIR}/*.hpp ${LIB_LUA_DIR}/*.ipp)

#list(REMOVE_ITEM LIB_LUA_SOURCE ${LIB_LUA_DIR}/lua.c ${LIB_LUA_DIR}/luac.c)

add_executable(${HELENA_APP} ${HELENA_APP_SOURCE} ${HELENA_APP_HEADERS})
#target_sources(${HELENA_APP} PRIVATE ${LIB_LUA_SOURCE} ${LIB_LUA_HEADERS})
#include_directories(${LIB_LUA_DIR})

source_group("Source" FILES ${HELENA_APP_SOURCE})
source_group("Headers" FILES ${HELENA_APP_HEADERS})

#source_group("Lua" FILES ${LIB_LUA_SOURCE})
#source_group("Lua" FILES ${LIB_LUA_HEADERS})

if(WIN32)
	# Ignore this is the flag I use them to get .pdb files
	# I use it to analyze code performance when i reverse engineer a binary file
	if(NOT MINGW)
		set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_APP}.pdb")
	endif()
endif()

file(GLOB_RECURSE HELENA_PROJECT_HEADERS
    ${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/*.h
    ${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/*.hpp
    ${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/*.ipp)

foreach(FILE ${HELENA_PROJECT_HEADERS})
    get_filename_component(ABSOLUTE_PATH ${FILE} ABSOLUTE)
    get_filename_component(PARENT_DIR ${ABSOLUTE_PATH} DIRECTORY)

    string(REPLACE ${HELENA_PROJECT_DIR} "" GROUP ${PARENT_DIR})
    string(REPLACE "/" "\\" GROUP ${GROUP})
    string(REGEX REPLACE "^(\\\\)" "" GROUP ${GROUP})
    source_group(${GROUP} FILES ${FILE})
endforeach()

target_sources(${HELENA_APP} PRIVATE ${HELENA_PROJECT_HEADERS})
target_link_libraries(${HELENA_APP} Helena::Helena)