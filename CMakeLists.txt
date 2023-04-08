cmake_minimum_required(VERSION 3.14)
project(HelenaFramework VERSION 1.0.2 LANGUAGES CXX)

#|--------------------------------
#| Helena Framework Options
#|--------------------------------
option(HELENA_FLAG_TEST             "Build and run test"    OFF)
option(HELENA_FLAG_EXAMPLES         "Build examples"        ON)
option(HELENA_FLAG_USE_BUNDLED_DEPS "Use bundled deps (e.g. fmt)" ON)
option(HELENA_FLAG_VIEW_HELENA      "Helena folder show in target project" ON)
option(HELENA_FLAG_BIN_DIR          "Enable bin directory of object and binary files" ON)

#|--------------------------------
#| Set default build type
#|--------------------------------
if(UNIX)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
    endif()
endif()

#|--------------------------------
#| Set supported build types
#|--------------------------------
set(HELENA_SUPPORTED_BUILD_TYPES Debug Release)

#|--------------------------------
#| Config compiler
#|--------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_SUPPRESS_REGENERATION OFF)
set(CMAKE_CONFIGURATION_TYPES "${HELENA_SUPPORTED_BUILD_TYPES}" CACHE STRING "" FORCE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeBuild")

#|--------------------------------
#| CMake modules
#|--------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

#|--------------------------------
#| Helena Framework Variables
#|--------------------------------
set(HELENA_PROJECT_NAME ${PROJECT_NAME})
set(HELENA_PROJECT_VERSION ${PROJECT_VERSION})
set(HELENA_PROJECT_FRAMEWORK_DIR "Helena")

if(CMAKE_VERSION VERSION_LESS 3.21)
  get_property(not_top DIRECTORY PROPERTY PARENT_DIRECTORY)
  if(NOT not_top)
    set(PROJECT_IS_TOP_LEVEL true)
  endif()
endif()

#|--------------------------------
#| Helena Framework Configuration
#|--------------------------------
set(HELENA_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(HELENA_PROJECT_BIN_DIR ${CMAKE_SOURCE_DIR}/Bin)
message(STATUS "Helena Project Dir: ${HELENA_PROJECT_DIR}")


if(DEFINED CMAKE_BUILD_TYPE)
    if(CMAKE_BUILD_TYPE IN_LIST HELENA_SUPPORTED_BUILD_TYPES)
        set(HELENA_BUILD_TYPE ${CMAKE_BUILD_TYPE})
	message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
    else()
        message(FATAL_ERROR "Error: unknown build type \"${CMAKE_BUILD_TYPE}\"")
    endif()
endif()

message(STATUS "Helena Binary Dir: ${HELENA_PROJECT_BIN_DIR}")
message(STATUS "Build ${HELENA_PROJECT_NAME} [${HELENA_PROJECT_VERSION}] [C++${CMAKE_CXX_STANDARD}] [${CMAKE_BUILD_TYPE}]")

#|---------------------------------------------
#| Helena Framework Interface target and alias
#|---------------------------------------------
add_library(Helena INTERFACE)
add_library(Helena::Helena ALIAS Helena)

#|--------------------------------
#| Helena Framework Dependencies
#|--------------------------------

if(NOT PROJECT_IS_TOP_LEVEL)
    if(HELENA_FLAG_VIEW_HELENA)
        macro(HELENA_FUNCTION_SOURCE_GROUP SourceFiles ConcatDir)
            foreach(FILE ${SourceFiles})
                get_filename_component(ABSOLUTE_PATH ${FILE} ABSOLUTE)
                get_filename_component(PARENT_DIR ${ABSOLUTE_PATH} DIRECTORY)

                string(REPLACE ${ConcatDir} "" GROUP ${PARENT_DIR})
                string(REPLACE "/" "\\" GROUP ${GROUP})
                string(REGEX REPLACE "^(\\\\)" "" GROUP ${GROUP})
                source_group(TREE ${ConcatDir} PREFIX ${GROUP} FILES ${FILE})
            endforeach()
        endmacro()

        file(GLOB_RECURSE HELENA_PROJECT_HEADERS
            ${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/*.h
            ${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/*.hpp
            ${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/*.ipp)
        HELENA_FUNCTION_SOURCE_GROUP("${HELENA_PROJECT_HEADERS}" "${HELENA_PROJECT_DIR}")
        target_sources(Helena INTERFACE ${HELENA_PROJECT_HEADERS})
    endif()
endif()

target_include_directories(Helena INTERFACE ${HELENA_PROJECT_DIR})
if(HELENA_FLAG_USE_BUNDLED_DEPS)
    target_include_directories(Helena INTERFACE ${HELENA_PROJECT_DIR}/Dependencies)
else()
    find_package(fmt REQUIRED)
    if(HELENA_FLAG_TEST)
        find_package(GTest REQUIRED)
    endif()
    target_link_libraries(Helena INTERFACE fmt::fmt-header-only)
endif()

#|--------------------------------
#| Helena Framework Build
#|--------------------------------
if(NOT UNIX AND NOT WIN32)
    message(FATAL_ERROR "Current platform is not supported!")
endif()

if(UNIX OR MINGW)
	target_compile_options(Helena INTERFACE -Wall -Wextra -fvisibility=hidden
        $<$<CONFIG:Debug>:-O0 -g -ggdb>
        $<$<CONFIG:Release>:-O3>
	)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(Helena INTERFACE -fno-gnu-unique)
    if(MINGW)
        target_compile_options(Helena INTERFACE -Wno-interference-size)
    endif()
endif()

if(WIN32)
    if(MINGW)
        target_link_libraries(Helena INTERFACE -static -static-libgcc -static-libstdc++ winpthread)
    else()
        target_compile_options(Helena INTERFACE /Zc:preprocessor)
    endif()
    target_link_libraries(Helena INTERFACE winmm.lib dbghelp.lib ws2_32.lib iphlpapi.lib userenv.lib)
endif()

if(HELENA_FLAG_BIN_DIR)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${HELENA_PROJECT_BIN_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${HELENA_PROJECT_BIN_DIR}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${HELENA_PROJECT_BIN_DIR}")

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${HELENA_PROJECT_BIN_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${HELENA_PROJECT_BIN_DIR}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${HELENA_PROJECT_BIN_DIR}")

    link_directories(${HELENA_PROJECT_BIN_DIR})
endif()

#|--------------------------------
#| Build with test
#|--------------------------------
if(HELENA_FLAG_TEST)
    message(STATUS "Buiding tests ...")
    enable_testing()
    add_subdirectory(Test)
endif()

#|--------------------------------
#| Build with examples
#|--------------------------------
if(HELENA_FLAG_EXAMPLES)
    message(STATUS "Building examples...")
    add_subdirectory(Examples)
endif()
