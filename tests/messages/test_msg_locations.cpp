#include <gtest/gtest.h>
#include "messages/MsgLocations.h"

using namespace messages;

TEST(AddTest, MsgLocationRequestConstructor)
{
    unsigned int id = 103;
    unsigned int length = sizeof(MsgLocationsRequest);

    MsgLocationsRequest locations_request;

    EXPECT_EQ(locations_request.id, id);
    EXPECT_EQ(locations_request.length, length);
}

TEST(AddTest, MsgLocationResponseConstructor)
{
    unsigned int id = 104;
    unsigned int length = sizeof(MsgLocationsResponse);

    MsgLocationsResponse locations_response;

    EXPECT_EQ(locations_response.id, id);
    EXPECT_EQ(locations_response.length, length);
    EXPECT_EQ(locations_response.char_count, 0);
    EXPECT_EQ(locations_response.is_paginated, false);
}

TEST(AddTest, MsgLocationResponseAddLocations)
{
    MsgLocationsResponse locations_response;
    
    std::string location = "abcdefghijklmno"; //string of 15 length, in addition to the \n char it will fail on the 7th add

    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.char_count, 16);
    EXPECT_FALSE(locations_response.is_paginated);

    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.char_count, 32);
    EXPECT_FALSE(locations_response.is_paginated);

    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.char_count, 48);
    EXPECT_FALSE(locations_response.is_paginated);

    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.char_count, 64);
    EXPECT_FALSE(locations_response.is_paginated);

    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.char_count, 80);
    EXPECT_FALSE(locations_response.is_paginated);

    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.char_count, 96);
    EXPECT_FALSE(locations_response.is_paginated);

    //the size will remain unchanged and the pagnation will be set
    EXPECT_FALSE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.char_count, 96);
    EXPECT_TRUE(locations_response.is_paginated);

    //check the value of the string buffer
    EXPECT_EQ(locations_response.GetLocations(), "abcdefghijklmno;abcdefghijklmno;abcdefghijklmno;abcdefghijklmno;abcdefghijklmno;abcdefghijklmno;");
}

