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

TEST(AddTest, AddDesination)
{
    Location location_src("test location 1", 10);
    Location location_valid_dst("test location 2", 20);
    Location location_invalid_dst("test location 3", 30);

    location_src.AddDestination(&location_valid_dst); 

    EXPECT_TRUE(location_src.DestinationIsValid(&location_valid_dst));    
    EXPECT_FALSE(location_src.DestinationIsValid(&location_invalid_dst));

    auto valid_destinations = location_src.Destinations();
    EXPECT_EQ(&location_valid_dst, (valid_destinations.find(location_valid_dst.Name()))->second);
}