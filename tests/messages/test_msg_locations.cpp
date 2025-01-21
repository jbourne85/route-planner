#include <gtest/gtest.h>
#include "messages/MsgLocations.h"

using namespace messages;

TEST(AddTest, MsgLocationRequestConstructor)
{
    unsigned int id = 103;
    unsigned int length = sizeof(MsgHeaderData);

    MsgLocationsRequest locations_request;

    EXPECT_EQ(locations_request.Id(), id);
    EXPECT_EQ(locations_request.Length(), length);
}

TEST(AddTest, MsgLocationResponseConstructor)
{
    unsigned int id = 104;
    unsigned int length = sizeof(MsgHeaderData) + sizeof(MsgLocationsResponseData);

    MsgLocationsResponse locations_response;

    EXPECT_EQ(locations_response.Id(), id);
    EXPECT_EQ(locations_response.Length(), length);
    EXPECT_EQ(locations_response.GetData()->char_count, 0);
    EXPECT_EQ(locations_response.GetData()->is_paginated, false);
}

TEST(AddTest, MsgLocationResponseAddLocations)
{
    MsgLocationsResponse locations_response;
    
    //Add strings of 15 length, in addition to the \n char it will fail on the 7th add
    std::string location = "c19ecpm2lq98vj9";
    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.GetData()->char_count, 16);
    EXPECT_FALSE(locations_response.GetData()->is_paginated);

    location = "jhh0cbadeifukpk";
    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.GetData()->char_count, 32);
    EXPECT_FALSE(locations_response.GetData()->is_paginated);

    location = "zdjubir1rz796oh";
    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.GetData()->char_count, 48);
    EXPECT_FALSE(locations_response.GetData()->is_paginated);

    location = "uhvik04r2ippd1d";
    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.GetData()->char_count, 64);
    EXPECT_FALSE(locations_response.GetData()->is_paginated);

    location = "c7n845q7j9c7odz";
    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.GetData()->char_count, 80);
    EXPECT_FALSE(locations_response.GetData()->is_paginated);
    
    location = "bhr472bpw4d0w89";
    EXPECT_TRUE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.GetData()->char_count, 96);
    EXPECT_FALSE(locations_response.GetData()->is_paginated);

    //the size will remain unchanged and the pagnation will be set
    location = "rlo9nx1gc08vuhg";
    EXPECT_FALSE(locations_response.AddLocation(location));
    EXPECT_EQ(locations_response.GetData()->char_count, 96);
    EXPECT_TRUE(locations_response.GetData()->is_paginated);

    //check the value of the locations list
    auto loocations_list = locations_response.GetLocations();
    EXPECT_EQ(loocations_list.size(), 6);
    EXPECT_EQ(loocations_list[0], "c19ecpm2lq98vj9");
    EXPECT_EQ(loocations_list[1], "jhh0cbadeifukpk");
    EXPECT_EQ(loocations_list[2], "zdjubir1rz796oh");
    EXPECT_EQ(loocations_list[3], "uhvik04r2ippd1d");
    EXPECT_EQ(loocations_list[4], "c7n845q7j9c7odz");
    EXPECT_EQ(loocations_list[5], "bhr472bpw4d0w89");
}

