cmake_minimum_required(VERSION 3.28)   # FetchContent SYSTEM + EXCLUDE_FROM_ALL + FIND_PACKAGE_ARGS
project(modern_cpp_tutorial VERSION 0.1.0
        DESCRIPTION "Modern C++ by exercise: a companion to the logger rebuild guide" LANGUAGES CXX)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Use a preset: cmake --preset clang-debug")
endif()

# Presets set the build type; this covers a bare `cmake -B build`.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Every executable lands in build/<preset>/bin/. The tests registered by cmake/TutExercises.cmake name
# them by that path, so a test can be defined before its target has ever been built.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# ON by default, unlike most projects: an exercise is built by `ctest`, which shows compiler output only
# when the build fails. A warning that is not an error would never be seen.
option(TUT_WARNINGS_AS_ERRORS "Treat compiler and clang-tidy warnings as errors" ON)
option(TUT_ENABLE_CLANG_TIDY "Run clang-tidy on our sources while compiling" OFF)
set(TUT_SANITIZERS "" CACHE STRING "Sanitizers for our targets, e.g. address;undefined or thread")

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM AND NOT CMAKE_CXX_COMPILER_LAUNCHER)   # a launcher the user chose wins
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# Point clangd (in any editor) at the most recently configured preset.
# Windows allows symlinks only in Developer Mode (or as admin), so a failure is a note, not an error.
file(CREATE_LINK ${PROJECT_BINARY_DIR}/compile_commands.json
     ${PROJECT_SOURCE_DIR}/compile_commands.json RESULT link_result SYMBOLIC)
if(NOT link_result EQUAL 0)
    message(STATUS "Could not link compile_commands.json into the source tree (${link_result}). "
                   "On Windows, turn on Developer Mode to allow symlinks.")
endif()

enable_testing()

include(${PROJECT_SOURCE_DIR}/cmake/TutOptions.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/TutDependencies.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/TutExercises.cmake)

add_subdirectory(support)

# One line per chapter, in reading order. After adding one, configure again (cmake --preset <name>).
add_subdirectory(chapters/00-warm-up)
add_subdirectory(chapters/01-views)

tut_finish()
