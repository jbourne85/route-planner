#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comms/TcpMsgMatch.h"
#include "messages/MsgFactory.h"
#include "messages/MockMsgHeader.h"

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
        
        msg->serialize(msg_buffer);

        std::copy(msg_buffer.begin(), msg_buffer.begin() + length, std::ostream_iterator<char>(os));
    }

    std::unique_ptr<TcpMsgMatch> tcp_msg_match;
    std::unique_ptr<MsgFactory> msg_factory;
};

TEST_F(TcpMsgMatchTest, MatchNotFoundWhenBufferIsIncomplete) {
    
    boost::asio::streambuf buffer;
    auto header = msg_factory->header();

    populate_buffer(buffer, header, 3); //write a portion of the message to the buffer

    auto begin = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_begin(buffer.data()));
    auto end = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_end(buffer.data()));

    auto result = tcp_msg_match->match_found(begin, end);

    //Expect the message not to be found
    EXPECT_FALSE(result.second);
    EXPECT_TRUE(nullptr == tcp_msg_match->get_match());
}

TEST_F(TcpMsgMatchTest, MatchFoundWhenBufferContainsCompleteMessage) {
    boost::asio::streambuf buffer;
    auto header = msg_factory->header();

    populate_buffer(buffer, header, header->length); //write the whole header

    auto begin = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_begin(buffer.data()));
    auto end = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_end(buffer.data()));

    auto result = tcp_msg_match->match_found(begin, end);

    // Expect a message to be found
    EXPECT_TRUE(result.second);
    EXPECT_TRUE(nullptr != tcp_msg_match->get_match());
}


TEST_F(TcpMsgMatchTest, HandleIncorrectMessageFound) {
    boost::asio::streambuf buffer;
    
    auto header = MsgHeader::MsgPointer(new MsgHeader(0, sizeof(MsgHeader))); //create a message with an invalid id

    populate_buffer(buffer, header, header->length); //write the whole header

    auto begin = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_begin(buffer.data()));
    auto end = TcpMsgMatch::MsgBufferIterator(boost::asio::buffers_end(buffer.data()));

    auto result = tcp_msg_match->match_found(begin, end);

    //Expect the message not to be found
    EXPECT_FALSE(result.second);
    EXPECT_TRUE(nullptr == tcp_msg_match->get_match());
}
