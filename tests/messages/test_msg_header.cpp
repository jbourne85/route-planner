#include <gtest/gtest.h>
#include "messages/MsgHeader.h"

using namespace messages;

TEST(AddTest, MsgHeaderConstructor)
{
    unsigned int id = 100;
    unsigned int length = sizeof(MsgHeader);

    MsgHeader header(id, length);

    EXPECT_EQ(header.id, id);
    EXPECT_EQ(header.length, 32);
}

TEST(AddTest, MsgHeaderSerialiseDeserialise)
{
    unsigned int id = 100;
    unsigned int length = sizeof(MsgHeader);

    MsgHeader header(id, length);

    std::vector<char> buffer;
    header.Serialize(buffer);
    
    EXPECT_EQ(buffer.size(), length);

    MsgHeader empty_header(0, length);

    empty_header.Deserialize(buffer);

    EXPECT_EQ(empty_header.id, id);
    EXPECT_EQ(empty_header.length, length);    
    EXPECT_EQ(empty_header.timestamp, header.timestamp);
}

TEST(AddTest, MsgHeaderDeserialiseNotEnoughData)
{
    unsigned int id = 100;
    unsigned int length = sizeof(MsgHeader);

    MsgHeader header(id, length);

    std::vector<char> buffer(4);   //create an empty buffer of 4 bytes

    MsgHeader empty_header(id, length);

    unsigned int bytes_deserialized = empty_header.Deserialize(buffer);

    EXPECT_EQ(bytes_deserialized, 4);
    EXPECT_EQ(empty_header.id, 100); 
    EXPECT_NE(empty_header.length, 0);    
    EXPECT_NE(empty_header.timestamp, 0);
}