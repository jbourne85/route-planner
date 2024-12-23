#include <gtest/gtest.h>
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

using namespace messages;

TEST(AddTest, MsgHeaderCreate)
{
    auto header = construct_message(MSG_HEADER_ID);

    EXPECT_EQ(header->id, MSG_HEADER_ID);
    EXPECT_EQ(header->length, sizeof(MsgHeader));
}

TEST(AddTest, MsgStatusRequestCreate)
{
    auto status_request = construct_message(MSG_STATUS_REQUEST_ID);

    EXPECT_EQ(status_request->id, MSG_STATUS_REQUEST_ID);
    EXPECT_EQ(status_request->length, sizeof(MsgStatusRequest));
}

TEST(AddTest, MsgStatusResponseCreate)
{
    auto status_response = construct_message(MSG_STATUS_RESPONSE_ID);

    EXPECT_EQ(status_response->id, MSG_STATUS_RESPONSE_ID);
    EXPECT_EQ(status_response->length, sizeof(MsgStatusResponse));
}

TEST(AddTest, MsgNoMatchCreate)
{
    auto msg = construct_message(999);

    EXPECT_TRUE(msg == nullptr);
}
