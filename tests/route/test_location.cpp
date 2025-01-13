#include <gtest/gtest.h>
#include "route/Location.h"

using namespace route;

TEST(AddTest, LocationConstructor)
{
    Location* const location = new Location("test location", 10);
    
    EXPECT_EQ(location->Name(), "test location");
    EXPECT_EQ(location->Cost(), 10);
    EXPECT_EQ(location->Destinations().size(), 0);

    delete location;
}

TEST(AddTest, AddDesination)
{
    Location* const location_src = new Location("test location 1", 10);
    Location* const location_valid_dst = new Location("test location 2", 20);
    Location* const location_invalid_dst = new Location("test location 3", 30);

    location_src->AddDestination(location_valid_dst); 

    EXPECT_TRUE(location_src->DestinationIsValid(location_valid_dst));    
    EXPECT_FALSE(location_src->DestinationIsValid(location_invalid_dst));

    auto valid_destinations = location_src->Destinations();
    EXPECT_EQ(location_valid_dst, (valid_destinations.find(location_valid_dst->Name()))->second);

    delete location_src;
    delete location_valid_dst;
    delete location_invalid_dst;
}