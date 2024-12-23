#include <gtest/gtest.h>
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

using namespace messages;

TEST(AddTest, MsgHeaderCreate)
{
    MsgFactory factory;
    auto header = factory.create(MSG_HEADER_ID);

    EXPECT_EQ(header->id, MSG_HEADER_ID);
    EXPECT_EQ(header->length, sizeof(MsgHeader));
}

TEST(AddTest, MsgStatusRequestCreate)
{
    MsgFactory factory;
    auto status_request = factory.create(MSG_STATUS_REQUEST_ID);

    EXPECT_EQ(status_request->id, MSG_STATUS_REQUEST_ID);
    EXPECT_EQ(status_request->length, sizeof(MsgStatusRequest));
}

TEST(AddTest, MsgStatusResponseCreate)
{
    MsgFactory factory;
    auto status_response = factory.create(MSG_STATUS_RESPONSE_ID);

    EXPECT_EQ(status_response->id, MSG_STATUS_RESPONSE_ID);
    EXPECT_EQ(status_response->length, sizeof(MsgStatusResponse));
}

TEST(AddTest, MsgNoMatchCreate)
{
    MsgFactory factory;
    auto msg = factory.create(999);

    EXPECT_TRUE(msg == nullptr);
}
