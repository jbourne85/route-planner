#include <gtest/gtest.h>
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "messages/MsgLocations.h"
#include "messages/MsgStatus.h"

using namespace messages;

class MsgFactoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        msg_factory = std::make_unique<MsgFactory>();
    }

    std::unique_ptr<MsgFactory> msg_factory;
};

TEST_F(MsgFactoryTest, MsgHeaderCreate)
{
    auto header = msg_factory->Header();

    EXPECT_EQ(header->id, MSG_HEADER_ID);
    EXPECT_EQ(header->length, sizeof(MsgHeader));
}

TEST_F(MsgFactoryTest, MsgHeaderCreateById)
{
    auto header = msg_factory->Create(MSG_HEADER_ID);

    EXPECT_EQ(header->id, MSG_HEADER_ID);
    EXPECT_EQ(header->length, sizeof(MsgHeader));
}

TEST_F(MsgFactoryTest, MsgStatusRequestCreate)
{
    auto status_request = msg_factory->Create(MSG_STATUS_REQUEST_ID);

    EXPECT_EQ(status_request->id, MSG_STATUS_REQUEST_ID);
    EXPECT_EQ(status_request->length, sizeof(MsgStatusRequest));
    EXPECT_NE(MsgHeader::GetDerivedType<MsgStatusRequest>(status_request), nullptr);
}

TEST_F(MsgFactoryTest, MsgStatusResponseCreate)
{
    auto status_response = msg_factory->Create(MSG_STATUS_RESPONSE_ID);

    EXPECT_EQ(status_response->id, MSG_STATUS_RESPONSE_ID);
    EXPECT_EQ(status_response->length, sizeof(MsgStatusResponse));
    EXPECT_NE(MsgHeader::GetDerivedType<MsgStatusResponse>(status_response), nullptr);
}

TEST_F(MsgFactoryTest, MsgLocationsRequestCreate)
{
    auto locations_request = msg_factory->Create(MSG_LOCATIONS_REQUEST_ID);

    EXPECT_EQ(locations_request->id, MSG_LOCATIONS_REQUEST_ID);
    EXPECT_EQ(locations_request->length, sizeof(MsgLocationsRequest));
    EXPECT_NE(MsgHeader::GetDerivedType<MsgLocationsRequest>(locations_request), nullptr);
}

TEST_F(MsgFactoryTest, MsgLocationsResponseCreate)
{
    auto locations_response = msg_factory->Create(MSG_LOCATIONS_RESPONSE_ID);

    EXPECT_EQ(locations_response->id, MSG_LOCATIONS_RESPONSE_ID);
    EXPECT_EQ(locations_response->length, sizeof(MsgLocationsResponse));
    EXPECT_NE(MsgHeader::GetDerivedType<MsgLocationsResponse>(locations_response), nullptr);
}

TEST_F(MsgFactoryTest, MsgNoMatchCreate)
{
    auto msg = msg_factory->Create(0);

    EXPECT_TRUE(msg == nullptr);
}

TEST_F(MsgFactoryTest, MaxMsgLength)
{
    std::vector msg_ids = {
        MSG_HEADER_ID, 
        MSG_STATUS_REQUEST_ID, 
        MSG_STATUS_RESPONSE_ID, 
        MSG_LOCATIONS_REQUEST_ID, 
        MSG_LOCATIONS_RESPONSE_ID
    };

    size_t max_message_size = 0;

    //Get the largest current message
    for (auto it = msg_ids.begin(); it != msg_ids.end(); ++it) {
        if (msg_factory->Create(*it)->length > max_message_size) {
            max_message_size = msg_factory->Create(*it)->length;
        }
    }

    EXPECT_NE(max_message_size, 0);
    EXPECT_EQ(msg_factory->MaxLength(), max_message_size);
}