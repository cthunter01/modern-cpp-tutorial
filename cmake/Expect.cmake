# Script mode (cmake -P), driven by the tests that cmake/TutExercises.cmake registers. It runs a build or a
# program and checks that the outcome is the expected one, including the outcomes that are failures: a file
# that must not compile, an experiment that must crash, a stub whose tests must not pass.
#
#   -DMODE=BUILD  -DBUILD_DIR=<dir> -DTARGET=<target> -DRESULT=<ok|fail> [-DCONTROL=<target>] [-DEXPECT_FILE=<file>]
#                 [-DSOURCE=<file to name in messages>]
#       Builds TARGET. RESULT=fail passes only when the build fails. CONTROL is built first and must
#       succeed: it is the same source without the offending line, so that a typo cannot pass for a
#       correct rejection.
#   -DMODE=RUN    -DEXE=<path> -DRESULT=<zero|nonzero|timeout> [-DARGS=a|b] [-DTIMEOUT=<s>] [-DRUNS=<n>]
#                 [-DEXPECT_FILE=<file>] [-DSTDOUT_FILE=<file>]
#       Runs EXE. STDOUT_FILE demands exactly that standard output. RESULT=timeout passes only when the
#       program is still running after TIMEOUT seconds (a hang). RUNS demands the outcome n times in a row.
#   -DMODE=STUB   -DBUILD_DIR=<dir> -DTARGET=<target> -DEXE=<path> [-DMAY_PASS=Suite.Case|...] [-DFIXIT=1]
#       The inverse check for an untouched exercise: it must build, its tests must fail, they must say
#       NOT IMPLEMENTED, and no test case may pass. A case that passes on a stub checks nothing. FIXIT is
#       for an exercise that starts from wrong code instead of a stub: it only has to fail.
#   -DMODE=TIDY   -DBUILD_DIR=<dir> -DFILE=<source> -DCLANG_TIDY=<program> -DCONFIG=<.clang-tidy> -DEXPECT_FILE=<file>
#       Runs clang-tidy on FILE and checks its diagnostics.
#   -DMODE=ABSENT -DFILE=<path>
#       FILE must not exist (a design problem starts from no file at all).
#
# An EXPECT_FILE holds one literal fragment per line. Every fragment must appear in the output (standard
# output and standard error together), in the order given. Leading and trailing blanks are ignored, and so
# are empty lines. Fragments are literal text, not regular expressions: compiler diagnostics are full of
# <>()[] and would need escaping.

cmake_minimum_required(VERSION 3.28)   # a cmake -P script does not inherit the project's policies

# Diagnostics are matched as text: no translated messages, no typographic quotes.
set(ENV{LC_ALL} C)

function(require)
    foreach(var IN LISTS ARGN)
        if(NOT DEFINED ${var} OR "${${var}}" STREQUAL "")
            message(FATAL_ERROR "Expect.cmake MODE=${MODE} requires -D${var}=...")
        endif()
    endforeach()
endfunction()

# strip_colour(<text> <out-var>): the build trees force coloured diagnostics, which would split fragments.
string(ASCII 27 esc)
function(strip_colour text out_var)
    string(REGEX REPLACE "${esc}\\[[0-9;]*[A-Za-z]" "" plain "${text}")
    set(${out_var} "${plain}" PARENT_SCOPE)
endfunction()

# check_fragments(<output> <expect-file> <what>)
function(check_fragments output expect_file what)
    if(NOT EXISTS "${expect_file}")
        message(FATAL_ERROR "expectation file ${expect_file} does not exist")
    endif()
    file(READ "${expect_file}" spec)
    set(rest "${output}")
    set(checked 0)
    while(NOT "${spec}" STREQUAL "")
        string(FIND "${spec}" "\n" newline)
        if(newline EQUAL -1)
            set(line "${spec}")
            set(spec "")
        else()
            string(SUBSTRING "${spec}" 0 ${newline} line)
            math(EXPR next "${newline} + 1")
            string(SUBSTRING "${spec}" ${next} -1 spec)
        endif()
        string(STRIP "${line}" line)
        if("${line}" STREQUAL "")
            continue()
        endif()
        string(FIND "${rest}" "${line}" at)
        if(at EQUAL -1)
            message(FATAL_ERROR "${what}: expected to find, in this order, the lines of ${expect_file}.\n"
                                "Not found: ${line}\n--- output ---\n${output}")
        endif()
        string(LENGTH "${line}" length)
        math(EXPR cut "${at} + ${length}")
        string(SUBSTRING "${rest}" ${cut} -1 rest)
        math(EXPR checked "${checked} + 1")
    endwhile()
    if(checked EQUAL 0)
        message(FATAL_ERROR "${expect_file} holds no fragment: it would accept any output")
    endif()
endfunction()

# build_target(<target> <rc-var> <output-var>)
function(build_target target rc_var out_var)
    execute_process(COMMAND "${CMAKE_COMMAND}" --build "${BUILD_DIR}" --target "${target}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
    strip_colour("${out}" out)
    set(${rc_var} "${rc}" PARENT_SCOPE)
    set(${out_var} "${out}" PARENT_SCOPE)
endfunction()

require(MODE)
# Lists arrive as a|b: a semicolon would be split again on every hop through a CMake function.
foreach(var ARGS MAY_PASS)
    if(DEFINED ${var})
        string(REPLACE "|" ";" ${var} "${${var}}")
    endif()
endforeach()

if(MODE STREQUAL "BUILD")
    require(BUILD_DIR TARGET RESULT)
    if(DEFINED CONTROL AND NOT "${CONTROL}" STREQUAL "")
        build_target("${CONTROL}" rc out)
        if(NOT rc EQUAL 0)
            message(FATAL_ERROR "The part of this file that is valid code does not compile yet, so nothing can be "
                                "said about the part that must be rejected.\n--- output ---\n${out}")
        endif()
    endif()
    set(subject "${TARGET}")
    if(DEFINED SOURCE AND NOT "${SOURCE}" STREQUAL "")
        set(subject "${SOURCE}")
    endif()
    build_target("${TARGET}" rc out)
    if(RESULT STREQUAL "ok")
        if(NOT rc EQUAL 0)
            message(FATAL_ERROR "${subject} must compile, and does not.\n--- output ---\n${out}")
        endif()
    elseif(RESULT STREQUAL "fail")
        if(rc EQUAL 0)
            message(FATAL_ERROR "${subject} must NOT compile once TUT_BREAK is defined, and it does: the "
                                "compiler accepts the lines under #ifdef TUT_BREAK, which this exercise is "
                                "supposed to make it reject.")
        endif()
    else()
        message(FATAL_ERROR "MODE=BUILD: RESULT must be ok or fail, not '${RESULT}'")
    endif()
    if(DEFINED EXPECT_FILE AND NOT "${EXPECT_FILE}" STREQUAL "")
        check_fragments("${out}" "${EXPECT_FILE}" "building ${TARGET}")
    endif()

elseif(MODE STREQUAL "RUN")
    require(EXE RESULT)
    if(NOT EXISTS "${EXE}")
        message(FATAL_ERROR "${EXE} does not exist: build its target first")
    endif()
    if(NOT DEFINED TIMEOUT OR "${TIMEOUT}" STREQUAL "")
        set(TIMEOUT 60)
    endif()
    if(NOT DEFINED RUNS OR "${RUNS}" STREQUAL "")
        set(RUNS 1)
    endif()
    foreach(run RANGE 1 ${RUNS})
        if(DEFINED STDOUT_FILE AND NOT "${STDOUT_FILE}" STREQUAL "")
            execute_process(COMMAND "${EXE}" ${ARGS} TIMEOUT ${TIMEOUT}
                RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err)
            file(READ "${STDOUT_FILE}" wanted)
            if(NOT "${out}" STREQUAL "${wanted}")
                message(FATAL_ERROR "The standard output of ${EXE} is not the content of ${STDOUT_FILE}.\n"
                                    "--- stdout ---\n${out}\n--- expected ---\n${wanted}\n--- stderr ---\n${err}")
            endif()
            set(out "${out}${err}")
        else()
            # One variable for both pipes: the two streams are merged in the order they were produced.
            execute_process(COMMAND "${EXE}" ${ARGS} TIMEOUT ${TIMEOUT}
                RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
        endif()
        strip_colour("${out}" out)
        set(timed_out FALSE)
        if("${rc}" MATCHES "timeout")
            set(timed_out TRUE)
        endif()
        if(RESULT STREQUAL "zero")
            if(NOT "${rc}" STREQUAL "0")
                message(FATAL_ERROR "${EXE} must exit with 0; it ended with: ${rc}\n--- output ---\n${out}")
            endif()
        elseif(RESULT STREQUAL "nonzero")
            if("${rc}" STREQUAL "0" OR timed_out)
                message(FATAL_ERROR "${EXE} must fail; it ended with: ${rc}\n--- output ---\n${out}")
            endif()
        elseif(RESULT STREQUAL "timeout")
            if(NOT timed_out)
                message(FATAL_ERROR "${EXE} must still be running after ${TIMEOUT} s (a hang); it ended with: "
                                    "${rc}\n--- output ---\n${out}")
            endif()
        else()
            message(FATAL_ERROR "MODE=RUN: RESULT must be zero, nonzero or timeout, not '${RESULT}'")
        endif()
        if(DEFINED EXPECT_FILE AND NOT "${EXPECT_FILE}" STREQUAL "")
            check_fragments("${out}" "${EXPECT_FILE}" "running ${EXE} (run ${run} of ${RUNS})")
        endif()
    endforeach()

elseif(MODE STREQUAL "STUB")
    require(BUILD_DIR TARGET EXE)
    build_target("${TARGET}" rc out)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "The untouched exercise ${TARGET} must build, and does not.\n--- output ---\n${out}")
    endif()
    execute_process(COMMAND "${EXE}" --gtest_color=no TIMEOUT 120
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
    if("${rc}" STREQUAL "0")
        message(FATAL_ERROR "The tests of the untouched exercise ${TARGET} pass: they check nothing.\n"
                            "--- output ---\n${out}")
    endif()
    if(NOT FIXIT)
        string(FIND "${out}" "NOT IMPLEMENTED" at)
        if(at EQUAL -1)
            message(FATAL_ERROR "The untouched exercise ${TARGET} fails, but not by saying NOT IMPLEMENTED.\n"
                                "--- output ---\n${out}")
        endif()
        # Every case GoogleTest reports as passed must be on the allowed list.
        string(REGEX MATCHALL "\\[       OK \\] [A-Za-z0-9_./]+" passed "${out}")
        foreach(entry IN LISTS passed)
            string(REGEX REPLACE "^\\[       OK \\] " "" case "${entry}")
            if(NOT case IN_LIST MAY_PASS)
                message(FATAL_ERROR "The case ${case} passes on the untouched exercise ${TARGET}: it checks "
                                    "nothing (or list it under STUB_MAY_PASS).\n--- output ---\n${out}")
            endif()
        endforeach()
    endif()

elseif(MODE STREQUAL "TIDY")
    require(BUILD_DIR FILE CLANG_TIDY CONFIG EXPECT_FILE)
    execute_process(
        COMMAND "${CLANG_TIDY}" -p "${BUILD_DIR}" "--config-file=${CONFIG}"
                --extra-arg=-Wno-unknown-warning-option "${FILE}"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE out)
    strip_colour("${out}" out)
    check_fragments("${out}" "${EXPECT_FILE}" "clang-tidy on ${FILE}")

elseif(MODE STREQUAL "ABSENT")
    require(FILE)
    if(EXISTS "${FILE}")
        message(FATAL_ERROR "${FILE} exists. A design problem starts from no file: this one must not be "
                            "committed (the reference lives under solutions/).")
    endif()

else()
    message(FATAL_ERROR "Expect.cmake: unknown MODE '${MODE}'")
endif()
