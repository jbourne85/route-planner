#include <gtest/gtest.h>
#include "route/Location.h"

using namespace route;

TEST(AddTest, LocationConstructor)
{
    Location location("test location", 10);
    
    EXPECT_EQ(location.Name(), "test location");
    EXPECT_EQ(location.Cost(), 10);
    EXPECT_EQ(location.Destinations().size(), 0);
}