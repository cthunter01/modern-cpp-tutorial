# What the exercises share: TUT_TODO and the instrumented types the tests look through.
add_library(tut_support INTERFACE)
add_library(tut::support ALIAS tut_support)
target_include_directories(tut_support INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(tut_support INTERFACE cxx_std_23)

# The tutorial's own machinery, checked. Part of the default build: `cmake --workflow --preset dev` on a
# fresh clone proves that the compiler, GoogleTest and the support code work before any exercise is tried.
add_executable(tut_selfcheck selfcheck/todo_tests.cpp)
target_link_libraries(tut_selfcheck PRIVATE tut::support GTest::gtest_main)
tut_configure_target(tut_selfcheck)
add_test(NAME selfcheck COMMAND tut_selfcheck --gtest_brief=1)
set_tests_properties(selfcheck PROPERTIES LABELS selfcheck ENVIRONMENT "${TUT_TEST_ENVIRONMENT}")

# See tidy_canary/canary.h.
if(TUT_TREE STREQUAL "tidy")
    add_library(tut_tidy_canary OBJECT EXCLUDE_FROM_ALL tidy_canary/canary.cpp)
    target_link_libraries(tut_tidy_canary PRIVATE tut::support)
    tut_configure_target(tut_tidy_canary)
    _tut_expect_test(tidy_canary LABELS selfcheck
        ARGS -DMODE=BUILD -DBUILD_DIR=${PROJECT_BINARY_DIR} -DTARGET=tut_tidy_canary -DRESULT=fail
        -DEXPECT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tidy_canary/canary.expect)
endif()
