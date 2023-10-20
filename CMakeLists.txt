cmake_minimum_required(VERSION 3.14)

set(HELENA_VERSION_REGEX "#define HELENA_VERSION_.*[ \t]+(.+)")
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/Helena/Helena.hpp" HELENA_VERSION REGEX ${HELENA_VERSION_REGEX})
list(TRANSFORM HELENA_VERSION REPLACE ${HELENA_VERSION_REGEX} "\\1")
string(JOIN "." HELENA_VERSION ${HELENA_VERSION})

project(HelenaFramework VERSION ${HELENA_VERSION} LANGUAGES CXX)

#|--------------------------------
#| Helena Framework Options
#|--------------------------------
option(HELENA_FLAG_TEST             "Build and run test"    OFF)
option(HELENA_FLAG_EXAMPLES         "Build examples"        ON)
option(HELENA_FLAG_VIEW_HELENA      "Helena folder show in target project" OFF)
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
set(HELENA_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(HELENA_PROJECT_BIN_DIR "${CMAKE_SOURCE_DIR}/Bin")
message(STATUS "Helena Project Dir: ${HELENA_PROJECT_DIR}")

if(HELENA_FLAG_BIN_DIR)
	message(STATUS "Helena Project Bin Dir: ${HELENA_PROJECT_BIN_DIR}")
endif()

if(DEFINED CMAKE_BUILD_TYPE)
    if(CMAKE_BUILD_TYPE IN_LIST HELENA_SUPPORTED_BUILD_TYPES)
        set(HELENA_BUILD_TYPE ${CMAKE_BUILD_TYPE})
	message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
    else()
        message(FATAL_ERROR "Error: unknown build type \"${CMAKE_BUILD_TYPE}\"")
    endif()
endif()

message(STATUS "Build ${HELENA_PROJECT_NAME} [${HELENA_PROJECT_VERSION}] [C++${CMAKE_CXX_STANDARD}] [${CMAKE_BUILD_TYPE}]")

#|---------------------------------------------
#| Helena Framework Interface target and alias
#|---------------------------------------------
add_library(Helena INTERFACE)
add_library(Helena::Helena ALIAS Helena)

target_include_directories(Helena INTERFACE ${HELENA_PROJECT_DIR})
target_compile_features(Helena INTERFACE cxx_std_20)

target_include_directories(Helena INTERFACE
	$<BUILD_INTERFACE:${HELENA_PROJECT_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(HELENA_FLAG_VIEW_HELENA)
	set(HELENA_PROJECT_HEADERS
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Helena.hpp"

        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Engine/Engine.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Engine/Engine.ipp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Engine/Events.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Engine/Log.hpp"

        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Platform/Assert.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Platform/Compiler.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Platform/Defines.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Platform/Linux/Linux.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Platform/Platform.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Platform/Processor.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Platform/Windows/Windows.hpp"

        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Add.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/AnyOf.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Arguments.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Cacheline.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Conditional.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Constness.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Constructible.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/FNV1a.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Function.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Identity.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/NameOf.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/PowerOf2.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Remove.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/SameAll.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/SameAs.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/ScopedEnum.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Select.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Specialization.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/TypeCounter.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/TypeIndex.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/Underlying.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Traits/UniqueTypes.hpp"

        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/Allocators.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/Any.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/BasicLogger.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/BasicLoggerDefines.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/BenchmarkScoped.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/CompressedPair.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/DateTime.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/Delegate.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/EncryptedString.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/FixedBuffer.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/Hash.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/LocationString.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/Monostate.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/Mutex.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/Overloads.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/RWLock.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/ReferencePointer.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/SourceLocation.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/Spinlock.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/SPSCVector.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/StateMachine.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/Subsystems.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/System.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/TaskScheduler.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/TimeSpan.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/UniqueIndexer.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/VectorAny.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/VectorKVAny.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Types/VectorUnique.hpp"

        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Util/Cast.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Util/Format.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Util/Function.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Util/Sleep.hpp"
        "${HELENA_PROJECT_DIR}/${HELENA_PROJECT_FRAMEWORK_DIR}/Util/String.hpp"
	)

	target_sources(Helena INTERFACE ${HELENA_PROJECT_HEADERS})
endif()

#|--------------------------------
#| Helena Framework Dependencies
#|--------------------------------
if(HELENA_FLAG_TEST)
    find_package(GTest REQUIRED)
endif()

#|--------------------------------
#| Helena Framework Build
#|--------------------------------
if(NOT UNIX AND NOT WIN32)
    message(FATAL_ERROR "Current platform is not supported!")
endif()

if(UNIX OR MINGW)
	target_compile_options(Helena INTERFACE -Wall -Wextra #-fvisibility=hidden
        $<$<CONFIG:Debug>:-O0 -g -ggdb>
        $<$<CONFIG:Release>:-O3>
	)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(Helena INTERFACE -Wno-interference-size)
endif()

if(WIN32)
    if(MINGW)
        #-static -static-libgcc -static-libstdc++
        target_link_libraries(Helena INTERFACE winpthread)
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
#| Helena Frameowk Util functions
#|--------------------------------

# This function groups the header files of the framework into the directory "Helena" in your Project
# The function has no effect if the HELENA_FLAG_VIEW_HELENA flag is disabled.
function(HELENA_SOURCE_PRETTY)
	if(HELENA_FLAG_VIEW_HELENA)
		foreach(FILE ${HELENA_PROJECT_HEADERS})
			source_group(TREE "${HELENA_PROJECT_DIR}" FILES "${FILE}")
		endforeach()
	endif()
endfunction()

# Group your own files
# Example:
# HELENA_SOURCE_GLOB(APP_SOURCE "${CMAKE_CURRENT_LIST_DIR}" cpp hpp ipp h)
# HELENA_SOURCE_GROUP("${CMAKE_CURRENT_LIST_DIR}" "${APP_SOURCE}")
function(HELENA_SOURCE_GROUP ARG_PROJECT_SOURCE_DIR ARG_PROJECT_SOURCE_FILES)
    foreach(FILE ${ARG_PROJECT_SOURCE_FILES})
        source_group(TREE "${ARG_PROJECT_SOURCE_DIR}" FILES "${FILE}")
    endforeach()
endfunction()

# Search for files recursively with the given extensions in the given directory
# Example: HELENA_SOURCE_GLOB(APP_SOURCE "${CMAKE_CURRENT_LIST_DIR}" cpp hpp ipp h)
function(HELENA_SOURCE_GLOB ARG_PROJECT_SOURCE_OUT ARG_PROJECT_SOURCE_DIR)
	set(TEMP_FILE_LIST "")
	foreach(FILE_EXT ${ARGN})
		file(GLOB_RECURSE TEMP_FILE "${ARG_PROJECT_SOURCE_DIR}/*.${FILE_EXT}")
		list(APPEND TEMP_FILE_LIST ${TEMP_FILE})
	endforeach()
    set(${ARG_PROJECT_SOURCE_OUT} ${TEMP_FILE_LIST} PARENT_SCOPE)
endfunction()

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