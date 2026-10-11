# The functions a chapter's CMakeLists.txt is written with. A chapter directory looks like this:
#
#   chapters/05-slug/
#     examples/     e01_name.cpp [e01_name.out]     programs the prose quotes       tut_example()
#     experiments/  x01_name.cpp x01_name.expect    "Break it on purpose"           tut_experiment()
#     exercises/    d01_name.h                      drill stubs; the reader also    tut_drill()
#                                                   creates p01_name.h here         tut_problem()
#     tests/        d01_name_tests.cpp ...          test sources for both
#   solutions/05-slug/                              the same file names as exercises/
#
# Names: targets and tests of the reader's own files start with ch<NN>_ (ch05_d01, ch05_p01), those of the
# reference solutions with sol<NN>_. Examples are eg<NN>_<MM>, experiments x<NN>_<MM>. The prefixes are
# disjoint on purpose: `ctest -R ch05` selects the reader's chapter 5 and nothing else.
#
# Only examples are part of the default build. Everything else is built one target at a time, by the test
# that needs it: an unfinished exercise must never stop another one from building.
#
# How a test finds its program: every test of an exercise has a second test, <name>.build, as its fixture.
# That one runs `cmake --build <dir> --target <name>`, so `ctest -R ch05_d01` compiles first and a stale
# binary is never run. Two builds in one directory at the same time would corrupt it, hence the lock.

include_guard(GLOBAL)

set(TUT_EXPECT_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/Expect.cmake)

# What this build tree can show. Experiments and inverse checks are registered only where their outcome is
# the one the chapter quotes: a plain tree for ordinary behaviour, a sanitizer tree for a sanitizer report.
set(TUT_TREE plain)
if("address" IN_LIST TUT_SANITIZERS)
    set(TUT_TREE asan)
elseif("thread" IN_LIST TUT_SANITIZERS)
    set(TUT_TREE tsan)
elseif(TUT_ENABLE_CLANG_TIDY)
    set(TUT_TREE tidy)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(TUT_COMPILER gcc)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(TUT_COMPILER clang)
else()
    set(TUT_COMPILER other)
endif()

# Set on the test, not in a preset, so that `ctest --test-dir build/asan` behaves the same.
set(TUT_TEST_ENVIRONMENT "")
if(TUT_TREE STREQUAL "asan")
    set(TUT_TEST_ENVIRONMENT
        "ASAN_OPTIONS=detect_stack_use_after_return=1:strict_string_checks=1" "UBSAN_OPTIONS=print_stacktrace=1")
elseif(TUT_TREE STREQUAL "tsan")
    set(TUT_TEST_ENVIRONMENT "TSAN_OPTIONS=halt_on_error=1")
endif()

add_custom_target(exercises)     # every exercise as the reader has it
add_custom_target(solutions)     # every reference solution
add_custom_target(experiments)   # every experiment that is expected to build
set_property(GLOBAL PROPERTY TUT_MANIFEST "")

# _tut_manifest(<kind> <id> <file>...): one line of build/<preset>/tut-manifest.tsv, read by tools/.
function(_tut_manifest kind id)
    set(files "")
    foreach(file IN LISTS ARGN)
        file(RELATIVE_PATH relative ${PROJECT_SOURCE_DIR} ${file})
        list(APPEND files ${relative})
    endforeach()
    list(JOIN files "\t" joined)
    set_property(GLOBAL APPEND_STRING PROPERTY TUT_MANIFEST "${kind}\t${id}\t${joined}\n")
endfunction()

# _tut_expect_file(<out-var> <stem>): the expectation that applies to this tree and compiler, most
# specific first: <stem>.<tree>.<compiler>.expect, <stem>.<tree>.expect, <stem>.<compiler>.expect,
# <stem>.expect. Empty when there is none.
function(_tut_expect_file out_var stem)
    set(found "")
    foreach(candidate
            ${stem}.${TUT_TREE}.${TUT_COMPILER}.expect ${stem}.${TUT_TREE}.expect
            ${stem}.${TUT_COMPILER}.expect ${stem}.expect)
        if(EXISTS ${candidate})
            set(found ${candidate})
            break()
        endif()
    endforeach()
    set(${out_var} "${found}" PARENT_SCOPE)
endfunction()

# _tut_built_test(<name> LABELS <label>... [TIMEOUT <s>] COMMAND <command>...)
# A test that needs target <name> built: <name>.build is its fixture.
function(_tut_built_test name)
    cmake_parse_arguments(arg "" "TIMEOUT" "LABELS;COMMAND" ${ARGN})
    if(NOT arg_TIMEOUT)
        set(arg_TIMEOUT 60)
    endif()
    add_test(NAME ${name}.build COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR} --target ${name})
    set_tests_properties(${name}.build PROPERTIES
        FIXTURES_SETUP ${name} RESOURCE_LOCK tut_build TIMEOUT 900 LABELS "${arg_LABELS}")
    add_test(NAME ${name} COMMAND ${arg_COMMAND})
    set_tests_properties(${name} PROPERTIES
        FIXTURES_REQUIRED ${name} TIMEOUT ${arg_TIMEOUT} LABELS "${arg_LABELS}"
        ENVIRONMENT "${TUT_TEST_ENVIRONMENT}")
endfunction()

# _tut_expect_test(<name> LABELS <label>... ARGS <Expect.cmake argument>...)
# A test that is one run of Expect.cmake. It may build, so it takes the build lock.
function(_tut_expect_test name)
    cmake_parse_arguments(arg "" "TIMEOUT" "LABELS;ARGS" ${ARGN})
    if(NOT arg_TIMEOUT)
        set(arg_TIMEOUT 900)
    endif()
    add_test(NAME ${name} COMMAND ${CMAKE_COMMAND} ${arg_ARGS} -P ${TUT_EXPECT_SCRIPT})
    set_tests_properties(${name} PROPERTIES
        RESOURCE_LOCK tut_build TIMEOUT ${arg_TIMEOUT} LABELS "${arg_LABELS}"
        ENVIRONMENT "${TUT_TEST_ENVIRONMENT}")
endfunction()

# _tut_gtest_target(<target> <include-dir> [LEARNER | SOLUTION] SOURCES <file>...)
function(_tut_gtest_target target include_dir)
    cmake_parse_arguments(arg "LEARNER;SOLUTION" "" "SOURCES" ${ARGN})
    add_executable(${target} EXCLUDE_FROM_ALL ${arg_SOURCES})
    # Exactly one of exercises/ and solutions/<chapter>/. The test source lives in tests/, away from both:
    # an #include "x.h" looks in the including file's own directory first, and would always find a stub
    # that sat next to it.
    target_include_directories(${target} PRIVATE ${include_dir})
    target_link_libraries(${target} PRIVATE tut::support GTest::gtest_main)
    if(arg_LEARNER)
        tut_configure_target(${target} LEARNER)
    else()
        tut_configure_target(${target})
    endif()
    if(arg_SOLUTION)
        # The same test source is compiled twice. clangd should see it once, with the reader's headers.
        set_target_properties(${target} PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
    endif()
endfunction()

# _tut_compile_checks(<id> <include-dir> <aggregate> LABELS <label>... [LEARNER] [SOLUTION]
#                     [COMPILES <file>...] [REJECTS <file>...])
# COMPILES: tests/<file> must compile (test <id>.c1, .c2, ...). It holds what cannot be asked at run time:
#   static_asserts, a call that used not to compile.
# REJECTS: tests/<file> must compile as it is and must NOT compile with -DTUT_BREAK (test <id>.r1, ...).
#   The lines under #ifdef TUT_BREAK are the call the exercise has to make the compiler reject; the rest
#   of the file is the control. An expectation file next to it, <file stem>[.gcc|.clang].expect, names the
#   diagnostic.
function(_tut_compile_checks id include_dir aggregate)
    cmake_parse_arguments(arg "LEARNER;SOLUTION" "" "LABELS;COMPILES;REJECTS" ${ARGN})
    # No clang-tidy on these files: they are scaffolding around a header that the exercise's main test
    # already puts through it.

    set(index 0)
    foreach(file IN LISTS arg_COMPILES)
        math(EXPR index "${index} + 1")
        set(target ${id}_c${index})
        add_library(${target} OBJECT EXCLUDE_FROM_ALL ${TUT_CHAPTER_DIR}/tests/${file})
        target_include_directories(${target} PRIVATE ${include_dir})
        target_link_libraries(${target} PRIVATE tut::support)
        tut_configure_target(${target} NO_TIDY)
        if(arg_SOLUTION)
            set_target_properties(${target} PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
            add_dependencies(${aggregate} ${target})
        endif()
        file(RELATIVE_PATH shown ${PROJECT_SOURCE_DIR} ${TUT_CHAPTER_DIR}/tests/${file})
        _tut_expect_test(${id}.c${index} LABELS ${arg_LABELS}
            ARGS -DMODE=BUILD -DBUILD_DIR=${PROJECT_BINARY_DIR} -DTARGET=${target} -DRESULT=ok
            -DSOURCE=${shown})
        if(arg_LEARNER AND TUT_TREE STREQUAL "plain")
            # The inverse check: on the untouched stub this file must not compile.
            string(REPLACE "ch${TUT_CHAPTER_ID}_" "stub${TUT_CHAPTER_ID}_" stub_id ${id})
            _tut_expect_test(${stub_id}.c${index} LABELS stub ${TUT_CHAPTER_LABEL}
                ARGS -DMODE=BUILD -DBUILD_DIR=${PROJECT_BINARY_DIR} -DTARGET=${target} -DRESULT=fail)
        endif()
    endforeach()

    set(index 0)
    foreach(file IN LISTS arg_REJECTS)
        math(EXPR index "${index} + 1")
        set(target ${id}_r${index})
        foreach(variant control broken)
            set(variant_target ${target})
            if(variant STREQUAL "control")
                set(variant_target ${target}_control)
            endif()
            add_library(${variant_target} OBJECT EXCLUDE_FROM_ALL ${TUT_CHAPTER_DIR}/tests/${file})
            target_include_directories(${variant_target} PRIVATE ${include_dir})
            target_link_libraries(${variant_target} PRIVATE tut::support)
            set_target_properties(${variant_target} PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
            tut_configure_target(${variant_target} NO_TIDY)
            if(variant STREQUAL "broken")
                target_compile_definitions(${variant_target} PRIVATE TUT_BREAK)
            elseif(arg_SOLUTION)
                add_dependencies(${aggregate} ${variant_target})
            endif()
        endforeach()
        cmake_path(GET file STEM LAST_ONLY stem)
        _tut_expect_file(expect ${TUT_CHAPTER_DIR}/tests/${stem})
        file(RELATIVE_PATH shown ${PROJECT_SOURCE_DIR} ${TUT_CHAPTER_DIR}/tests/${file})
        _tut_expect_test(${id}.r${index} LABELS ${arg_LABELS}
            ARGS -DMODE=BUILD -DBUILD_DIR=${PROJECT_BINARY_DIR} -DTARGET=${target} -DCONTROL=${target}_control
            -DRESULT=fail "-DEXPECT_FILE=${expect}" -DSOURCE=${shown})
    endforeach()
endfunction()

# tut_chapter(<NN> <slug> [LABELS <label>...])
# The first call in a chapter's CMakeLists.txt. LABELS are added to every test of the chapter (for example
# `concurrency`, which the author-side checks repeat under ThreadSanitizer).
macro(tut_chapter id slug)
    cmake_parse_arguments(TUT_CHAPTER "" "" "LABELS" ${ARGN})
    set(TUT_CHAPTER_ID ${id})
    set(TUT_CHAPTER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(TUT_SOLUTION_DIR ${PROJECT_SOURCE_DIR}/solutions/${id}-${slug})
    set(TUT_CHAPTER_LABEL ch${id} ${TUT_CHAPTER_LABELS})
    add_custom_target(ch${id})    # the reader's exercises of this chapter
    add_custom_target(sol${id})   # its reference solutions
    add_dependencies(exercises ch${id})
    add_dependencies(solutions sol${id})
endmacro()

# tut_example(<eNN> <name> [OUTPUT] [ARGS <arg>...] [NO_TIDY])
# examples/<eNN>_<name>.cpp, a complete program the prose quotes. Part of the default build. It must exit
# with 0; with OUTPUT its standard output must be exactly examples/<eNN>_<name>.out, the file the chapter
# shows as the program's output.
function(tut_example eid name)
    cmake_parse_arguments(arg "OUTPUT;NO_TIDY" "" "ARGS" ${ARGN})
    string(SUBSTRING ${eid} 1 -1 number)
    set(target eg${TUT_CHAPTER_ID}_${number})
    set(stem ${TUT_CHAPTER_DIR}/examples/${eid}_${name})
    add_executable(${target} ${stem}.cpp)
    target_link_libraries(${target} PRIVATE tut::support)
    if(arg_NO_TIDY)
        tut_configure_target(${target} NO_TIDY)
    else()
        tut_configure_target(${target})
    endif()
    list(JOIN arg_ARGS "|" program_args)
    set(output "")
    if(arg_OUTPUT)
        set(output "-DSTDOUT_FILE=${stem}.out")
    endif()
    add_test(NAME ${target}
        COMMAND ${CMAKE_COMMAND} -DMODE=RUN -DEXE=$<TARGET_FILE:${target}> -DRESULT=zero "-DARGS=${program_args}"
                ${output} -P ${TUT_EXPECT_SCRIPT})
    set_tests_properties(${target} PROPERTIES
        TIMEOUT 60 LABELS "example;${TUT_CHAPTER_LABEL}" ENVIRONMENT "${TUT_TEST_ENVIRONMENT}")
    _tut_manifest(example ${target} ${stem}.cpp)
endfunction()

# tut_experiment(<xNN> <name> <COMPILE_ERROR | PRINTS | FAILS | HANGS <seconds> | TIDY>
#                [REQUIRES <asan|tsan>] [ONLY <gcc|clang>] [CONTROL] [NO_WERROR] [RUNS <n>] [ARGS <arg>...])
# experiments/<xNN>_<name>.cpp, the program behind a "Break it on purpose" section, and the outcome the
# chapter quotes, which is the expectation file (see _tut_expect_file for its name).
#   COMPILE_ERROR  the file must not compile. With CONTROL the file is first compiled without -DTUT_BREAK
#                  and must compile: put the offending lines under #ifdef TUT_BREAK
#   PRINTS         it runs, exits with 0 and prints the expected text
#   FAILS          it runs and ends badly: a non-zero exit, an abort, a sanitizer report
#   HANGS <s>      it is still running after <s> seconds
#   TIDY           clang-tidy reports the expected diagnostic (registered in the tidy tree)
# Without REQUIRES the experiment is registered in the plain trees (no sanitizer, no clang-tidy); with it,
# only in that sanitizer's tree. ONLY restricts it to one compiler, for the cases where the two disagree
# (one refuses to compile what the other lets through). The same file can be declared several times.
function(tut_experiment xid name kind)
    cmake_parse_arguments(arg "CONTROL;NO_WERROR" "REQUIRES;ONLY;RUNS" "ARGS" ${ARGN})
    set(seconds "")
    if(kind STREQUAL "HANGS")
        list(POP_FRONT arg_UNPARSED_ARGUMENTS seconds)
    endif()
    string(SUBSTRING ${xid} 1 -1 number)
    set(target x${TUT_CHAPTER_ID}_${number})
    set(stem ${TUT_CHAPTER_DIR}/experiments/${xid}_${name})
    _tut_manifest(experiment ${target} ${stem}.cpp)

    # The target exists in every tree, so that the reader can build and run it wherever he likes.
    if(NOT TARGET ${target})
        set(options NO_TIDY)
        if(arg_NO_WERROR)
            list(APPEND options NO_WERROR)
        endif()
        add_executable(${target} EXCLUDE_FROM_ALL ${stem}.cpp)
        target_link_libraries(${target} PRIVATE tut::support)
        target_compile_definitions(${target} PRIVATE TUT_BREAK)
        tut_configure_target(${target} ${options})
        add_executable(${target}_control EXCLUDE_FROM_ALL ${stem}.cpp)
        target_link_libraries(${target}_control PRIVATE tut::support)
        set_target_properties(${target}_control PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
        tut_configure_target(${target}_control ${options})
    endif()

    set(wanted_tree plain)
    if(kind STREQUAL "TIDY")
        set(wanted_tree tidy)
    elseif(arg_REQUIRES)
        set(wanted_tree ${arg_REQUIRES})
    endif()
    if(NOT TUT_TREE STREQUAL wanted_tree)
        return()
    endif()
    if(arg_ONLY AND NOT TUT_COMPILER STREQUAL arg_ONLY)
        return()
    endif()

    _tut_expect_file(expect ${stem})
    if(expect STREQUAL "")
        message(FATAL_ERROR "tut_experiment(${xid} ${name} ${kind}): no expectation file ${stem}[.${TUT_TREE}]"
                            "[.${TUT_COMPILER}].expect. An experiment without one checks nothing.")
    endif()
    set(labels experiment ${TUT_CHAPTER_LABEL})
    list(JOIN arg_ARGS "|" program_args)
    set(exe ${PROJECT_BINARY_DIR}/bin/${target}${CMAKE_EXECUTABLE_SUFFIX})

    if(kind STREQUAL "COMPILE_ERROR")
        set(control "")
        if(arg_CONTROL)
            set(control -DCONTROL=${target}_control)
        endif()
        _tut_expect_test(${target} LABELS ${labels}
            ARGS -DMODE=BUILD -DBUILD_DIR=${PROJECT_BINARY_DIR} -DTARGET=${target} -DRESULT=fail ${control}
            -DEXPECT_FILE=${expect})
    elseif(kind STREQUAL "TIDY")
        _tut_expect_test(${target} LABELS ${labels}
            ARGS -DMODE=TIDY -DBUILD_DIR=${PROJECT_BINARY_DIR} -DFILE=${stem}.cpp -DCLANG_TIDY=${CLANG_TIDY_PROGRAM}
            -DCONFIG=${PROJECT_SOURCE_DIR}/.clang-tidy -DEXPECT_FILE=${expect})
    elseif(kind MATCHES "^(PRINTS|FAILS|HANGS)$")
        set(result zero)
        set(timeout 60)
        if(kind STREQUAL "FAILS")
            set(result nonzero)
        elseif(kind STREQUAL "HANGS")
            set(result timeout)
            set(timeout ${seconds})
        endif()
        add_dependencies(experiments ${target})
        _tut_built_test(${target} LABELS ${labels} TIMEOUT 300
            COMMAND ${CMAKE_COMMAND} -DMODE=RUN -DEXE=${exe} -DRESULT=${result} -DTIMEOUT=${timeout}
                    "-DARGS=${program_args}" "-DRUNS=${arg_RUNS}" -DEXPECT_FILE=${expect} -P ${TUT_EXPECT_SCRIPT})
    else()
        message(FATAL_ERROR "tut_experiment(${xid} ${name}): unknown kind '${kind}'")
    endif()
endfunction()

# _tut_exercise_tests(<id> <kind> LABELS <label>... [TIMEOUT <s>])
function(_tut_exercise_tests id)
    cmake_parse_arguments(arg "LEARNER" "TIMEOUT" "LABELS" ${ARGN})
    set(colour "")
    if(arg_LEARNER)
        set(colour --gtest_color=yes)   # ctest is not a terminal; the reader's eyes are
    endif()
    _tut_built_test(${id} LABELS ${arg_LABELS} TIMEOUT "${arg_TIMEOUT}"
        COMMAND ${PROJECT_BINARY_DIR}/bin/${id}${CMAKE_EXECUTABLE_SUFFIX} --gtest_brief=1 ${colour})
endfunction()

# tut_drill(<dNN> <name> [FIXIT] [COMPILES <file>...] [REJECTS <file>...] [STUB_MAY_PASS <Suite.Case>...]
#           [TIMEOUT <s>])
# A drill: exercises/<dNN>_<name>.h is the stub the reader edits, tests/<dNN>_<name>_tests.cpp its tests,
# solutions/<chapter>/<dNN>_<name>.h the reference. Creates
#   ch<NN>_<dNN>     the tests against the reader's file         label exercise
#   sol<NN>_<dNN>    the same tests against the reference        label solution
#   stub<NN>_<dNN>   the inverse check (plain trees): the untouched stub must fail every case, saying
#                    NOT IMPLEMENTED                             label stub
# FIXIT: the starting file is wrong code to be repaired, not a stub; it only has to fail its tests.
# COMPILES and REJECTS: see _tut_compile_checks.
function(tut_drill did name)
    cmake_parse_arguments(arg "FIXIT" "TIMEOUT" "COMPILES;REJECTS;STUB_MAY_PASS" ${ARGN})
    set(stem ${did}_${name})
    set(id ch${TUT_CHAPTER_ID}_${did})
    set(solution_id sol${TUT_CHAPTER_ID}_${did})
    set(test_source ${TUT_CHAPTER_DIR}/tests/${stem}_tests.cpp)
    set(exercise_dir ${TUT_CHAPTER_DIR}/exercises)
    foreach(required ${test_source} ${exercise_dir}/${stem}.h ${TUT_SOLUTION_DIR}/${stem}.h)
        if(NOT EXISTS ${required})
            message(FATAL_ERROR "tut_drill(${did} ${name}): ${required} does not exist")
        endif()
    endforeach()
    _tut_manifest(drill ${id} ${exercise_dir}/${stem}.h ${test_source} ${TUT_SOLUTION_DIR}/${stem}.h)

    _tut_gtest_target(${id} ${exercise_dir} LEARNER SOURCES ${test_source} ${exercise_dir}/${stem}.h)
    add_dependencies(ch${TUT_CHAPTER_ID} ${id})
    _tut_exercise_tests(${id} LEARNER LABELS exercise ${TUT_CHAPTER_LABEL} TIMEOUT "${arg_TIMEOUT}")
    _tut_compile_checks(${id} ${exercise_dir} ch${TUT_CHAPTER_ID} LEARNER LABELS exercise ${TUT_CHAPTER_LABEL}
        COMPILES ${arg_COMPILES} REJECTS ${arg_REJECTS})

    _tut_gtest_target(${solution_id} ${TUT_SOLUTION_DIR} SOLUTION
        SOURCES ${test_source} ${TUT_SOLUTION_DIR}/${stem}.h)
    add_dependencies(sol${TUT_CHAPTER_ID} ${solution_id})
    _tut_exercise_tests(${solution_id} LABELS solution ${TUT_CHAPTER_LABEL} TIMEOUT "${arg_TIMEOUT}")
    _tut_compile_checks(${solution_id} ${TUT_SOLUTION_DIR} sol${TUT_CHAPTER_ID} SOLUTION
        LABELS solution ${TUT_CHAPTER_LABEL} COMPILES ${arg_COMPILES} REJECTS ${arg_REJECTS})

    if(TUT_TREE STREQUAL "plain")
        list(JOIN arg_STUB_MAY_PASS "|" may_pass)
        set(fixit "")
        if(arg_FIXIT)
            set(fixit -DFIXIT=1)
        endif()
        _tut_expect_test(stub${TUT_CHAPTER_ID}_${did} LABELS stub ${TUT_CHAPTER_LABEL}
            ARGS -DMODE=STUB -DBUILD_DIR=${PROJECT_BINARY_DIR} -DTARGET=${id}
            -DEXE=${PROJECT_BINARY_DIR}/bin/${id}${CMAKE_EXECUTABLE_SUFFIX} "-DMAY_PASS=${may_pass}"
            ${fixit})
    endif()
endfunction()

# tut_problem(<pNN> <name> [REFERENCE_ONLY] [COMPILES <file>...] [REJECTS <file>...] [TIMEOUT <s>])
# A design problem. There is no starting file: the reader creates exercises/<pNN>_<name>.h (and, if he
# wants, exercises/<pNN>_<name>*.cpp, which are compiled in). Until that header exists, ch<NN>_<pNN> is a
# program that says which file to create. Once it does, the next build notices (the glob below is
# re-evaluated on every build) and ch<NN>_<pNN> becomes the test program, without running cmake by hand.
# REFERENCE_ONLY: nothing checks the reader's answer automatically; only the reference is built and tested.
function(tut_problem pid name)
    cmake_parse_arguments(arg "REFERENCE_ONLY" "TIMEOUT" "COMPILES;REJECTS" ${ARGN})
    set(stem ${pid}_${name})
    set(id ch${TUT_CHAPTER_ID}_${pid})
    set(solution_id sol${TUT_CHAPTER_ID}_${pid})
    set(test_source ${TUT_CHAPTER_DIR}/tests/${stem}_tests.cpp)
    set(exercise_dir ${TUT_CHAPTER_DIR}/exercises)
    foreach(required ${test_source} ${TUT_SOLUTION_DIR}/${stem}.h)
        if(NOT EXISTS ${required})
            message(FATAL_ERROR "tut_problem(${pid} ${name}): ${required} does not exist")
        endif()
    endforeach()
    _tut_manifest(problem ${id} ${test_source} ${TUT_SOLUTION_DIR}/${stem}.h)

    file(GLOB solution_sources CONFIGURE_DEPENDS ${TUT_SOLUTION_DIR}/${stem}*.cpp)
    _tut_gtest_target(${solution_id} ${TUT_SOLUTION_DIR} SOLUTION
        SOURCES ${test_source} ${TUT_SOLUTION_DIR}/${stem}.h ${solution_sources})
    add_dependencies(sol${TUT_CHAPTER_ID} ${solution_id})
    _tut_exercise_tests(${solution_id} LABELS solution ${TUT_CHAPTER_LABEL} TIMEOUT "${arg_TIMEOUT}")
    _tut_compile_checks(${solution_id} ${TUT_SOLUTION_DIR} sol${TUT_CHAPTER_ID} SOLUTION
        LABELS solution ${TUT_CHAPTER_LABEL} COMPILES ${arg_COMPILES} REJECTS ${arg_REJECTS})

    if(arg_REFERENCE_ONLY)
        return()
    endif()

    file(GLOB reader_files CONFIGURE_DEPENDS ${exercise_dir}/${stem}*)
    file(RELATIVE_PATH wanted ${PROJECT_SOURCE_DIR} ${exercise_dir}/${stem}.h)
    if(EXISTS ${exercise_dir}/${stem}.h)
        set(reader_sources ${reader_files})
        list(FILTER reader_sources INCLUDE REGEX "\\.(cpp|h)$")
        _tut_gtest_target(${id} ${exercise_dir} LEARNER SOURCES ${test_source} ${reader_sources})
        add_dependencies(ch${TUT_CHAPTER_ID} ${id})
    else()
        # A program that says what to do and fails, so that the reader gets a message, not a build error.
        add_executable(${id} EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/support/placeholder/not_started.cpp)
        target_compile_definitions(${id} PRIVATE TUT_TARGET="${id}" TUT_WANTED_FILE="${wanted}")
        set_target_properties(${id} PROPERTIES EXPORT_COMPILE_COMMANDS OFF)
        tut_configure_target(${id} NO_TIDY)
    endif()
    _tut_exercise_tests(${id} LEARNER LABELS exercise ${TUT_CHAPTER_LABEL} TIMEOUT "${arg_TIMEOUT}")
    _tut_compile_checks(${id} ${exercise_dir} ch${TUT_CHAPTER_ID} LEARNER LABELS exercise ${TUT_CHAPTER_LABEL}
        COMPILES ${arg_COMPILES} REJECTS ${arg_REJECTS})

    if(TUT_TREE STREQUAL "plain")
        add_test(NAME stub${TUT_CHAPTER_ID}_${pid}
            COMMAND ${CMAKE_COMMAND} -DMODE=ABSENT -DFILE=${exercise_dir}/${stem}.h -P ${TUT_EXPECT_SCRIPT})
        set_tests_properties(stub${TUT_CHAPTER_ID}_${pid} PROPERTIES LABELS "stub;${TUT_CHAPTER_LABEL}")
    endif()
endfunction()

# tut_finish(): the last call of the top-level CMakeLists.txt.
function(tut_finish)
    get_property(manifest GLOBAL PROPERTY TUT_MANIFEST)
    file(WRITE ${PROJECT_BINARY_DIR}/tut-manifest.tsv "${manifest}")
endfunction()
