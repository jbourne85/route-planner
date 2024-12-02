#include <gtest/gtest.h>
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

TEST(AddTest, msg_header_create)
{
    std::shared_ptr<MsgHeader> header = construct_message(MSG_HEADER_ID);

    EXPECT_EQ(header->id, MSG_HEADER_ID);
    EXPECT_EQ(header->length, sizeof(MsgHeader));
}

TEST(AddTest, msg_status_request_create)
{
    std::shared_ptr<MsgHeader> status_request = construct_message(MSG_STATUS_REQUEST_ID);

    EXPECT_EQ(status_request->id, MSG_STATUS_REQUEST_ID);
    EXPECT_EQ(status_request->length, sizeof(MsgStatusRequest));
}

TEST(AddTest, msg_status_response_create)
{
    std::shared_ptr<MsgHeader> status_response = construct_message(MSG_STATUS_RESPONSE_ID);

    EXPECT_EQ(status_response->id, MSG_STATUS_RESPONSE_ID);
    EXPECT_EQ(status_response->length, sizeof(MsgStatusResponse));
}

TEST(AddTest, Test_msg_no_match_create)
{
    std::shared_ptr<MsgHeader> msg = construct_message(999);

    EXPECT_TRUE(msg == nullptr);
}
