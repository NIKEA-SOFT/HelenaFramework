cmake_minimum_required(VERSION 3.14)
project(HelenaFramework VERSION 1.0.2 LANGUAGES CXX)

#|--------------------------------
#| Helena Framework Options
#|--------------------------------
option(HELENA_FLAG_TEST         "Build and run test"    OFF)
option(HELENA_FLAG_EXAMPLES     "Build examples"        ON)
option(HELENA_FLAG_COVERAGE     "GCC/Clang coverage"    OFF)

#|--------------------------------
#| Set default build type
#|--------------------------------
if(UNIX)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
    endif()
endif()


#|--------------------------------
#| Config compiler
#|--------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_SUPPRESS_REGENERATION OFF)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;" CACHE STRING "" FORCE)
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

#|--------------------------------
#| Helena Framework Configuration
#|--------------------------------
set(HELENA_PROJECT_DIR ${CMAKE_SOURCE_DIR})
set(HELENA_PROJECT_OUTDIR ${HELENA_PROJECT_DIR}/Bin/Bin)
message(STATUS "Helena Project Dir: ${HELENA_PROJECT_DIR}")

set(HELENA_PROJECT_BINDIR ${HELENA_PROJECT_DIR}/Bin)
if(DEFINED CMAKE_BUILD_TYPE)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(HELENA_BUILD_TYPE ${CMAKE_BUILD_TYPE})
        message(STATUS "Build type: Debug")
    elseif(CMAKE_BUILD_TYPE MATCHES Release)
        set(HELENA_BUILD_TYPE ${CMAKE_BUILD_TYPE})
        message(STATUS "Build type: Release")
    else()
    message(FATAL_ERROR "Error: unknown build type")
    endif()
endif()

message(STATUS "Helena Binary Dir: ${HELENA_PROJECT_OUTDIR}")
message(STATUS "Build ${HELENA_PROJECT_NAME} [${HELENA_PROJECT_VERSION}] [C++${CMAKE_CXX_STANDARD}] [${CMAKE_BUILD_TYPE}]")

#|--------------------------------
#| Helena Framework Dependencies
#|--------------------------------
include_directories(${HELENA_PROJECT_DIR})
include_directories(${HELENA_PROJECT_DIR}/Dependencies)

#|--------------------------------
#| Helena Framework Build
#|--------------------------------
if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING")
elseif(UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -ggdb -Wall -Wextra -D_DEBUG -fPIC")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -DNDEBUG -fPIC")

    add_definitions(-Wno-unused-parameter -Wno-unused-variable -Wno-unused-function)
    if(HELENA_FLAG_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_CXX_FLAGS_DEBUG "-lgcov --coverage")
        set(CMAKE_CXX_FLAGS_RELEASE "-lgcov --coverage")
        message(STATUS "Build coverage")
        add_compile_options(--coverage -O0)
    endif()
else()
    message(FATAL_ERROR "Current platform not support!")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${HELENA_PROJECT_OUTDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${HELENA_PROJECT_OUTDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${HELENA_PROJECT_OUTDIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${HELENA_PROJECT_OUTDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${HELENA_PROJECT_OUTDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${HELENA_PROJECT_OUTDIR}")

link_directories(${HELENA_PROJECT_OUTDIR})

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

#|--------------------------------
#| Build with coverage
#|--------------------------------
if(HELENA_FLAG_COVERAGE)
    find_package(codecov)
endif()
