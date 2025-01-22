#include <gtest/gtest.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include "messages/MsgHeader.h"

using namespace messages;

struct MockTestMsgData {
    char padding[1024];

    MockTestMsgData() {
        memset(padding, ' ', 1024);
    }

    void FillPadding(char padding_char) {
        memset(padding, padding_char, 1024);
    }

    void ExpectPadding(char padding_char) {
        for (size_t i = 0 ; i < 1024 ; ++i) {
            EXPECT_EQ(padding[i], padding_char);
        }
    }
};

class MockTestMsg : public MsgHeader {
public:
    MockTestMsgData msg;
    MockTestMsg(unsigned int id) : MsgHeader(id, sizeof(MockTestMsgData), (char* const)&msg) {}
};

class MsgHeaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        log4cxx::PropertyConfigurator::configure("log4cxx.properties");
    }
};

TEST_F(MsgHeaderTest, MsgHeaderConstructor)
{
    unsigned int id = 100;
    size_t length = sizeof(MsgHeader);

    MsgHeader header;

    EXPECT_EQ(header.Id(), id);
    EXPECT_EQ(header.Length(), 24);
}

TEST_F(MsgHeaderTest, MsgHeaderSerialiseDeserialise)
{
    unsigned int id = 999;
    size_t length = sizeof(MsgHeaderData) + sizeof(MockTestMsgData);

    MockTestMsg mock_msg(id);
    mock_msg.msg.FillPadding('a');

    std::vector<char> buffer;
    mock_msg.Serialize(buffer);
    
    EXPECT_EQ(buffer.size(), length);

    MockTestMsg empty_header(0);
    empty_header.msg.ExpectPadding(' ');

    empty_header.Deserialize(buffer);

    EXPECT_EQ(empty_header.Id(), id);
    EXPECT_EQ(empty_header.Length(), length);    
    EXPECT_EQ(empty_header.Timestamp(), mock_msg.Timestamp());
    empty_header.msg.ExpectPadding('a');
}

TEST_F(MsgHeaderTest, MsgHeaderDeserialiseNotEnoughDataForHeader)
{
    unsigned int id = 999;
    size_t length = sizeof(MsgHeaderData) + sizeof(MockTestMsgData);

    MockTestMsg mock_msg(id);

    std::vector<char> buffer(4);   //create an empty buffer of 4 bytes

    MockTestMsg empty_header(0);
    empty_header.msg.FillPadding('a');

    size_t bytes_deserialized = empty_header.Deserialize(buffer);

    EXPECT_EQ(bytes_deserialized, 0);
    EXPECT_EQ(empty_header.Id(), 0); 
    EXPECT_NE(empty_header.Length(), 0);    
    EXPECT_NE(empty_header.Timestamp(), 0);
    empty_header.msg.ExpectPadding('a');
}

TEST_F(MsgHeaderTest, MsgHeaderDeserialiseNotEnoughDataForBody)
{
    unsigned int id = 999;
    size_t length = sizeof(MsgHeaderData) + sizeof(MockTestMsgData);

    MockTestMsg mock_msg(id);

    std::vector<char> buffer(sizeof(MsgHeaderData) + 4);   //create an empty buffer of 4 bytes bigger than the header

    MockTestMsg empty_header(0);

    size_t bytes_deserialized = empty_header.Deserialize(buffer);

    EXPECT_EQ(bytes_deserialized, 24);
    EXPECT_EQ(empty_header.Id(), 0); 
    EXPECT_EQ(empty_header.Length(), 0);    
    EXPECT_EQ(empty_header.Timestamp(), 0);
    empty_header.msg.ExpectPadding(' ');
}