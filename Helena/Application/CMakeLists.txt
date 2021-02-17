#|--------------------------------
#| HF Application Project
#|--------------------------------
set(HELENA_APP Helena)
file(GLOB_RECURSE HELENA_APP_SOURCE *.cpp *.cc *.c)
file(GLOB_RECURSE HELENA_APP_HEADERS *.h *.hpp *.ipp)
add_executable(${HELENA_APP} ${HELENA_APP_SOURCE} ${HELENA_APP_HEADERS})

# lua
set(CMAKE_VERBOSE_MAKEFILE ON)
set(LUA_LIB_SRCS 
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lapi.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lcode.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lctype.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/ldebug.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/ldo.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/ldump.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lfunc.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lgc.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/llex.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lmem.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lobject.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lopcodes.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lparser.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lstate.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lstring.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/ltable.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/ltm.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lundump.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lvm.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lzio.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lauxlib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lbaselib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lcorolib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/ldblib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/liolib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lmathlib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/loadlib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/loslib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lstrlib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/ltablib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/lutf8lib.c"
    "${HELENA_PROJECT_DIR}/Dependencies/lua/src/linit.c"
)

add_library(lua_static STATIC ${LUA_LIB_SRCS})
set_target_properties(lua_static PROPERTIES OUTPUT_NAME "lua" LINKER_LANGUAGE CXX)
target_include_directories(lua_static PUBLIC "${HELENA_PROJECT_DIR}/Dependencies/lua/include")

# link lua
target_link_libraries(${HELENA_APP} lua)

# other...
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
