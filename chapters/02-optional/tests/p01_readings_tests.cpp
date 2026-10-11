#include "p01_readings.h"

#include <gtest/gtest.h>

// The tests fix a class, Readings, with Add(value), AddMissing() and Average(). They look at what
// Average() returns in two ways only: `if (average)` asks whether there is an average, and
// `*average` is the average. Which type does that, and what Readings stores, is yours to decide.

namespace
{

TEST(Readings, AveragesWhatWasDelivered)
{
    ch02::Readings readings;
    readings.Add(20.0);
    readings.Add(22.0);
    readings.Add(24.0);

    const auto average = readings.Average();

    if (!average)
    {
        FAIL() << "there are readings, so there is an average";
    }
    EXPECT_DOUBLE_EQ(*average, 22.0);
}

TEST(Readings, MissingReadingsDoNotCount)
{
    ch02::Readings readings;
    readings.Add(10.0);
    readings.AddMissing();
    readings.AddMissing();
    readings.Add(20.0);

    const auto average = readings.Average();

    if (!average)
    {
        FAIL() << "there are readings, so there is an average";
    }
    EXPECT_DOUBLE_EQ(*average, 15.0);  // not 7.5: a missing reading is not a reading of zero
}

TEST(Readings, AnAverageOfZeroIsAnAverage)
{
    ch02::Readings readings;
    readings.Add(-5.0);
    readings.Add(5.0);

    const auto average = readings.Average();

    // It froze, and the mean was exactly 0.0. The caller must be able to tell that from "no data".
    if (!average)
    {
        FAIL() << "there are readings, so there is an average";
    }
    EXPECT_DOUBLE_EQ(*average, 0.0);
}

TEST(Readings, NothingDeliveredMeansNoAverage)
{
    ch02::Readings onlyMissing;
    onlyMissing.AddMissing();
    onlyMissing.AddMissing();
    EXPECT_FALSE(onlyMissing.Average());

    const ch02::Readings untouched;
    EXPECT_FALSE(untouched.Average());
}

}  // namespace
