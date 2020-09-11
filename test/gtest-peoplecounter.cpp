// ---------------------------------------------------------------------------
// @file    gtest-peoplecounter.cpp
//
// @brieg   Google Tests for people couter system.
//
// @date    July 15, 2020
//
// @author  Gustavo Puche
// ---------------------------------------------------------------------------
#include <gtest/gtest.h>

#include <PeopleCounter.cpp>

#include <vector>

TEST(PeopleCounterTest, Distance)
{
	EXPECT_EQ(distance(0,0,1,1), sqrt(2));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
