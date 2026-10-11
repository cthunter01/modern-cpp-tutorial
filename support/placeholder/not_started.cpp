// Built in place of a design problem's tests for as long as the reader's file does not exist.
// TUT_TARGET and TUT_WANTED_FILE are set per target by tut_problem() in cmake/TutExercises.cmake.

#include <iostream>

int main()
{
    std::cout << TUT_TARGET << " is a design problem: it starts from no file at all.\n"
              << "Create " << TUT_WANTED_FILE << " and run this again.\n";
    return 1;
}
