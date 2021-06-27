#|--------------------------------
#| HF Application Project
#|--------------------------------
cmake_minimum_required(VERSION 3.9)

set(HELENA_APP Helena)
set(LIB_LUA_DIR ${HELENA_PROJECT_DIR}/Dependencies/lua)

project(${HELENA_APP})

file(GLOB_RECURSE HELENA_APP_SOURCE *.cpp *.cc *.c)
file(GLOB_RECURSE HELENA_APP_HEADERS *.h *.hpp *.ipp)

file(GLOB_RECURSE LIB_LUA_SOURCE ${LIB_LUA_DIR}/*.cpp ${LIB_LUA_DIR}/*.cc ${LIB_LUA_DIR}/*.c)
file(GLOB_RECURSE LIB_LUA_HEADERS ${LIB_LUA_DIR}/*.h ${LIB_LUA_DIR}/*.hpp ${LIB_LUA_DIR}/*.ipp)

list(REMOVE_ITEM LIB_LUA_SOURCE ${LIB_LUA_DIR}/lua.c ${LIB_LUA_DIR}/luac.c)

add_executable(${HELENA_APP} ${HELENA_APP_SOURCE} ${HELENA_APP_HEADERS})
target_sources(${HELENA_APP} PRIVATE ${LIB_LUA_SOURCE} ${LIB_LUA_HEADERS})
include_directories(${LIB_LUA_DIR})

source_group("Source" FILES ${HELENA_APP_SOURCE})
source_group("Headers" FILES ${HELENA_APP_HEADERS})

source_group("Lua" FILES ${LIB_LUA_SOURCE})
source_group("Lua" FILES ${LIB_LUA_HEADERS})

if(WIN32)
    set_target_properties(${HELENA_APP} PROPERTIES LINK_FLAGS "/DEBUG /PDBSTRIPPED:${HELENA_APP}.pdb")
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()
