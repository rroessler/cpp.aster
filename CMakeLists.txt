cmake_minimum_required(VERSION 3.28.3)

# - PROJECT INCLUDES - #

# get all the baseline items
include("cmake/prelude.cmake")

# - PROJECT SETUP - #

# declare the project for building
project(${ASTER_TARGET_NAME} VERSION ${ASTER_VERSION_SHORT} LANGUAGES CXX)

# ensuring a default build is set to release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()

# setting the master-project details
if(NOT DEFINED ASTER_MASTER_PROJECT)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(ASTER_MASTER_PROJECT ON)
    else()
        set(ASTER_MASTER_PROJECT OFF)
    endif()
endif()

# define the additional options to be used
option(ASTER_TESTING_ENABLED "Build testing suite" ${ASTER_MASTER_PROJECT})
option(ASTER_BENCHMARKS_ENABLED "Build benchmarks suite" ${ASTER_MASTER_PROJECT})
option(ASTER_INSTALL_ENABLED "Generate the install target" ${ASTER_MASTER_PROJECT})

# prepare the baseline library to be declared
add_library(${ASTER_TARGET_NAME} INTERFACE)
add_library(${ASTER_TARGET_NAME}::${ASTER_TARGET_NAME} ALIAS ${ASTER_TARGET_NAME})

# ensure we set the baseline standard as well
target_compile_features(${ASTER_TARGET_NAME} INTERFACE cxx_std_${ASTER_CXX_STANDARD})

# bind all the available includes as well
target_include_directories(${ASTER_TARGET_NAME} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

# - PROJECT INSTALLATION - #

# allow testing if possible
if(ASTER_TESTING_ENABLED)
    add_subdirectory("tests")
endif()

# only install if possible to do so
if(ASTER_INSTALL_ENABLED)
    include("cmake/install.cmake")
endif()
