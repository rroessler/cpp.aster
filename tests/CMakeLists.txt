# - TESTING INCLUDES - #

include(FetchContent)

# - TESTING DEPENDENCIES - #

# declare all the dependencies to be used
FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.11.0 GIT_SHALLOW TRUE)

# and actually get them before continuing
set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL TRUE)
set(CMAKE_SKIP_INSTALL_RULES ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Catch2)
set(CMAKE_SKIP_INSTALL_RULES OFF CACHE BOOL "" FORCE)
set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL FALSE)

# - TESTING SETUP - #

# declare some details about the testing setup
set(ASTER_TESTING_NAME "tests")

# Define all the available warnings now
set(ASTER_WARNINGS_MSVC /W4)
set(ASTER_WARNINGS_UNIX
    -Wall
    -Wextra
    -Wshadow
)

# declare the testing project
project(${ASTER_TESTING_NAME} VERSION ${ASTER_VERSION_SHORT} LANGUAGES CXX)

# - TESTING INSTALLATION - #

# ensure we make the executable available for use
add_executable(${ASTER_TESTING_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/source/tests.cpp")
target_compile_features(${ASTER_TESTING_NAME} PRIVATE cxx_std_${ASTER_CXX_STANDARD})
target_link_libraries(${ASTER_TESTING_NAME} PRIVATE ${ASTER_TARGET_NAME}::${ASTER_TARGET_NAME} Catch2::Catch2)
target_compile_options(${ASTER_TESTING_NAME} INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:${ASTER_WARNINGS_MSVC}>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:${ASTER_WARNINGS_UNIX}>
)
