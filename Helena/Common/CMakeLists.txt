#|--------------------------------
#| HF Application Project
#|--------------------------------
set(HELENA_COMMON "Common")
file(GLOB_RECURSE HELENA_COMMON_HEADERS *.h *.hpp *.ipp)
add_custom_target(${HELENA_COMMON} SOURCES ${HELENA_COMMON_HEADERS})
source_group("Headers" FILES ${HELENA_COMMON_HEADERS})