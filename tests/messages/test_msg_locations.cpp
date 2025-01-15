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
    unsigned int id = 103;
    unsigned int length = sizeof(MsgLocationsResponse);

    MsgLocationsResponse locations_response;

    EXPECT_EQ(locations_response.id, id);
    EXPECT_EQ(locations_response.length, length);
    EXPECT_EQ(locations_response.char_count, 0);
    EXPECT_EQ(locations_response.is_paginated, false);
}
