#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comms/TcpMsgMatch.h"
#include "messages/MsgFactory.h"

using namespace comms;
using namespace messages;

class TcpMsgMatchTest : public ::testing::Test {
protected:
    void SetUp() override {
        tcp_msg_match = std::make_unique<TcpMsgMatch>();
        msg_factory = std::make_unique<MsgFactory>();
    }

    void TearDown() override {
    }

    void populate_buffer(boost::asio::streambuf& buffer, MsgHeader::MsgPointer msg, unsigned int length) {
        std::vector<char> msg_buffer;
        std::ostream os(&buffer);
        
        msg->Serialize(msg_buffer);

        std::copy(msg_buffer.begin(), msg_buffer.begin() + length, std::ostream_iterator<char>(os));
    }

    std::unique_ptr<TcpMsgMatch> tcp_msg_match;
    std::unique_ptr<MsgFactory> msg_factory;
};

TEST_F(TcpMsgMatchTest, MatchNotFoundWhenBufferIsIncomplete) {
    
    boost::asio::streambuf buffer;
    auto header = msg_factory->Header();

    populate_buffer(buffer, header, 3); //write a portion of the message to the buffer

    auto begin = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_begin(buffer.data()));
    auto end = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_end(buffer.data()));

    auto result = tcp_msg_match->ProcessBuffer(begin, end);

    //Expect the message not to be found
    EXPECT_FALSE(result.second);
    EXPECT_TRUE(nullptr == tcp_msg_match->GetMsg());
}

TEST_F(TcpMsgMatchTest, MatchFoundWhenBufferContainsCompleteMessage) {
    boost::asio::streambuf buffer;
    auto header = msg_factory->Header();

    populate_buffer(buffer, header, header->length); //write the whole header

    auto begin = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_begin(buffer.data()));
    auto end = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_end(buffer.data()));

    auto result = tcp_msg_match->ProcessBuffer(begin, end);

    // Expect a message to be found
    EXPECT_TRUE(result.second);

    // Expect the found message to match the sent one
    auto match = tcp_msg_match->GetMsg();
    EXPECT_TRUE(nullptr != match);
    EXPECT_TRUE(header->id == match->id);
    EXPECT_TRUE(header->length == match->length);
    EXPECT_TRUE(header->timestamp == match->timestamp);
}


TEST_F(TcpMsgMatchTest, HandleIncorrectMessageFound) {
    boost::asio::streambuf buffer;
    
    auto header = MsgHeader::MsgPointer(new MsgHeader(0, sizeof(MsgHeader))); //create a message with an invalid id

    populate_buffer(buffer, header, header->length); //write the whole header

    auto begin = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_begin(buffer.data()));
    auto end = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_end(buffer.data()));

    auto result = tcp_msg_match->ProcessBuffer(begin, end);

    //Expect the message not to be found
    EXPECT_FALSE(result.second);
    EXPECT_TRUE(nullptr == tcp_msg_match->GetMsg());
}

TEST_F(TcpMsgMatchTest, MatchFoundOverMultipleChunks) {

    boost::asio::streambuf buffer;
    auto header = msg_factory->Header();

    populate_buffer(buffer, header, header->length); //write the whole header

    auto begin = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_begin(buffer.data()));

    EXPECT_FALSE(header->length % 4);   //check the header can be split into 4 chunks for the test

    size_t chunk_size = header->length / 4;
  
    //Expect the message not to be found
    auto result = tcp_msg_match->ProcessBuffer(begin, begin + chunk_size);  //chunk 1/4
    EXPECT_FALSE(result.second);
    EXPECT_TRUE(nullptr == tcp_msg_match->GetMsg());

    //Expect the message not to be found
    result = tcp_msg_match->ProcessBuffer(result.first, result.first + chunk_size); //chunk 2/4
    EXPECT_FALSE(result.second);
    EXPECT_TRUE(nullptr == tcp_msg_match->GetMsg());

    //Expect the message not to be found
    result = tcp_msg_match->ProcessBuffer(result.first, result.first + chunk_size); //chunk 3/4
    EXPECT_FALSE(result.second);
    EXPECT_TRUE(nullptr == tcp_msg_match->GetMsg());

    // Expect a message to be found
    result = tcp_msg_match->ProcessBuffer(result.first, result.first + chunk_size); //chunk 4/4
    EXPECT_TRUE(result.second);
    
    // Expect the found message to match the sent one
    auto match = tcp_msg_match->GetMsg();
    ASSERT_NE(nullptr,  match);
    EXPECT_TRUE(header->id == match->id);
    EXPECT_TRUE(header->length == match->length);
    EXPECT_TRUE(header->timestamp == match->timestamp);
}