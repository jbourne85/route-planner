#include <gtest/gtest.h>
#include "messages/MsgHeader.h"

using namespace messages;

TEST(AddTest, msg_header_constructor)
{
    unsigned int id = 100;
    unsigned int length = sizeof(MsgHeader);

    MsgHeader header(id, length);

    EXPECT_EQ(header.id, id);
    EXPECT_EQ(header.length, length);
}

TEST(AddTest, msg_header_serialise_deserialise_test)
{
    unsigned int id = 100;
    unsigned int length = sizeof(MsgHeader);

    MsgHeader header(id, length);

    std::vector<char> buffer;
    header.serialize(buffer);
    
    EXPECT_EQ(buffer.size(), length);

    MsgHeader empty_header(0, length);

    empty_header.deserialize(buffer);

    EXPECT_EQ(empty_header.id, id);
    EXPECT_EQ(empty_header.length, length);    
    EXPECT_EQ(empty_header.timestamp, header.timestamp);
}