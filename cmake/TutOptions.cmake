if("thread" IN_LIST TUT_SANITIZERS AND "address" IN_LIST TUT_SANITIZERS)
    message(FATAL_ERROR "TUT_SANITIZERS: 'thread' cannot be combined with 'address'")
endif()

if(TUT_SANITIZERS AND MSVC)
    message(FATAL_ERROR "TUT_SANITIZERS needs GCC or Clang (asan/tsan presets: Linux, macOS)")
endif()

if(TUT_ENABLE_CLANG_TIDY)
    # Xcode has no clang-tidy; on macOS it comes from Homebrew's llvm, which is not on PATH.
    find_program(CLANG_TIDY_PROGRAM clang-tidy
                 HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin REQUIRED)
    # GCC-only -W flags are unknown to clang-tidy's Clang frontend; don't report them.
    set(TUT_CLANG_TIDY_COMMAND ${CLANG_TIDY_PROGRAM} --extra-arg=-Wno-unknown-warning-option)
    if(TUT_WARNINGS_AS_ERRORS)
        list(APPEND TUT_CLANG_TIDY_COMMAND --warnings-as-errors=*)
    endif()
endif()

# tut_sanitize_target(<target>)
# Applies TUT_SANITIZERS to a target. tut_configure_target() does this for our targets. A dependency built
# from source needs it as well (see TutDependencies.cmake): a sanitizer has to instrument the whole program.
function(tut_sanitize_target target)
    if(NOT TUT_SANITIZERS)
        return()
    endif()
    list(JOIN TUT_SANITIZERS "," sanitizers)
    set(sanitize -fsanitize=${sanitizers})
    if("undefined" IN_LIST TUT_SANITIZERS)
        list(APPEND sanitize -fno-sanitize-recover=all)   # UB stops the program, so a test fails
    endif()
    target_compile_options(${target} PRIVATE ${sanitize} -fno-omit-frame-pointer)
    target_link_options(${target} PUBLIC ${sanitize})
    if("address" IN_LIST TUT_SANITIZERS)
        # libstdc++ annotates std::vector's spare capacity for ASan only on request (libc++ does it by
        # default), so reads past size() but within capacity() are caught. Ignored by libc++ and MSVC.
        # Every translation unit in the program must agree on this: a vector that annotated code poisoned and
        # unannotated code then grew is reported as a container-overflow that never happened.
        target_compile_definitions(${target} PRIVATE _GLIBCXX_SANITIZE_VECTOR)
    endif()
endfunction()

# tut_configure_target(<target> [LEARNER] [NO_WERROR] [NO_TIDY])
# Applies warnings, sanitizers and clang-tidy to one of *our* targets. Dependencies get the sanitizers only,
# through tut_sanitize_target(). Call it for every target you add.
#   LEARNER    the target compiles a file the reader is still writing: clang-tidy checks that fire on a stub
#              only because it is a stub are left out
#   NO_WERROR  warnings stay warnings (an experiment that is meant to run despite one)
#   NO_TIDY    never run clang-tidy on it (experiments, must-not-compile cases, "the old way" listings)
function(tut_configure_target target)
    cmake_parse_arguments(arg "LEARNER;NO_WERROR;NO_TIDY" "" "" ${ARGN})
    set(werror ${TUT_WARNINGS_AS_ERRORS})
    if(arg_NO_WERROR)
        set(werror OFF)
    endif()

    if(MSVC)   # cl, and clang-cl (which takes the same options)
        set(msvc_options
            /W4 /permissive- /utf-8 /Zc:__cplusplus $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
            # Off-by-default warnings, enabled at level 1: narrowing conversions (4242, 4254, 4826),
            # a missed override (4263), a non-virtual destructor (4265), always-false comparisons
            # (4287, 4296), pointer truncation (4311), comma/no-effect mistakes (4545-4555),
            # thread-unsafe statics (4640), string literal casts (4905, 4906), copy-init (4928).
            /w14242 /w14254 /w14263 /w14265 /w14287 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549
            /w14555 /w14640 /w14826 /w14905 /w14906 /w14928
            $<$<BOOL:${werror}>:/WX>)
        target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${msvc_options}>")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wnon-virtual-dtor
            -Wold-style-cast -Woverloaded-virtual -Wnull-dereference -Wdouble-promotion -Wformat=2
            -Wimplicit-fallthrough -Wcast-align
            $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast>
            $<$<BOOL:${werror}>:-Werror>)
        # Bounds-checked operator[] etc. in the standard library, ABI-compatible (unlike _GLIBCXX_DEBUG):
        # libstdc++ (Linux) and libc++ (macOS) each ignore the other's macro. MSVC's Debug STL checks itself.
        # Several experiments rely on it to turn silent undefined behaviour into an abort.
        target_compile_definitions(${target} PRIVATE
            $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>
            $<$<CONFIG:Debug>:_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE>)

        tut_sanitize_target(${target})
    endif()

    if(TUT_ENABLE_CLANG_TIDY AND NOT arg_NO_TIDY)
        set(tidy_command ${TUT_CLANG_TIDY_COMMAND})
        if(arg_LEARNER)
            # --checks is appended to the Checks of .clang-tidy. A stub's member functions touch no member
            # yet, and TUT_TODO throws out of functions the finished code declares noexcept.
            list(APPEND tidy_command
                 --checks=-readability-convert-member-functions-to-static,-bugprone-exception-escape)
        endif()
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${tidy_command}")
    endif()
endfunction()
