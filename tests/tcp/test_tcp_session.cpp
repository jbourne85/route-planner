#include <boost/asio.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comms/TcpSession.h"
#include "messages/MsgFactory.h"

using namespace comms;
using namespace messages;

class MockTcpSocket {
public:
    MockTcpSocket() {}
    MockTcpSocket(const MockTcpSocket& other) {}

    MOCK_METHOD(std::size_t, read_some, (const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec));
    MOCK_METHOD(std::size_t, write_some, (const boost::asio::const_buffers_1 &buffers, boost::system::error_code &ec));
};

class TcpSessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_socket = std::make_shared<MockTcpSocket>();
        tcp_session = std::make_unique<TcpSession<MockTcpSocket> >(*mock_socket);
        msg_factory = std::make_unique<MsgFactory>();
    }

    void TearDown() override {
    }

    std::shared_ptr<MockTcpSocket> mock_socket;
    std::unique_ptr<TcpSession<MockTcpSocket> > tcp_session;
    std::unique_ptr<MsgFactory> msg_factory;
};

TEST_F(TcpSessionTest, TestWaitForMessage) {  

    auto test_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), read_some).WillOnce([this, test_msg](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec) {
        std::vector<char> test_data;    
        test_msg->Serialize(test_data);

        char* dest = static_cast<char*>(buffers.data());
        std::memcpy(dest, test_data.data(), test_data.size());

        return test_data.size();
    });

    auto received_msg = tcp_session->WaitForMsg();

    EXPECT_NE(nullptr, received_msg);
    EXPECT_EQ(test_msg->id, received_msg->id);
    EXPECT_EQ(test_msg->length, received_msg->length);
    EXPECT_EQ(test_msg->timestamp, received_msg->timestamp);
}

TEST_F(TcpSessionTest, TestErrorWhileWaitForMessage) {  

    auto test_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), read_some).WillOnce([this, test_msg](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec) {
        ec = boost::asio::error::timed_out;        
        return 0;
    });

    auto received_msg = tcp_session->WaitForMsg();

    EXPECT_EQ(nullptr, received_msg);
}

TEST_F(TcpSessionTest, TestWaitForMessageInChunks) {  

    auto test_msg = msg_factory->Header();
    std::vector<char> test_data;    
    test_msg->Serialize(test_data);

    //test the header being read in chunks of 4 
    EXPECT_FALSE(test_msg->length % 4);         //check the header can be split into 4 chunks for the test
    size_t chunk_size = test_msg->length / 4;

    EXPECT_CALL(*(tcp_session->Socket()), read_some)
    .Times(4)  
    .WillOnce([this, test_data, chunk_size](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec) {
        char* dest = static_cast<char*>(buffers.data());
        std::memcpy(dest, test_data.data(), chunk_size);
        return chunk_size;
    })
    .WillOnce([this, test_data, chunk_size](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec) {
        char* dest = static_cast<char*>(buffers.data());
        std::memcpy(dest, test_data.data() + chunk_size * 1, chunk_size);
        return chunk_size;
    })
    .WillOnce([this, test_data, chunk_size](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec) {
        char* dest = static_cast<char*>(buffers.data());
        std::memcpy(dest, test_data.data() + chunk_size * 2, chunk_size);
        return chunk_size;
    })
    .WillOnce([this, test_data, chunk_size](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec) {
        char* dest = static_cast<char*>(buffers.data());
        std::memcpy(dest, test_data.data() + chunk_size * 3, chunk_size);
        return chunk_size;
    });

    auto received_msg = tcp_session->WaitForMsg();

    EXPECT_NE(nullptr, received_msg);
    EXPECT_EQ(test_msg->id, received_msg->id);
    EXPECT_EQ(test_msg->length, received_msg->length);
    EXPECT_EQ(test_msg->timestamp, received_msg->timestamp);
}

TEST_F(TcpSessionTest, TestWriteMessage) {  

    auto test_msg = msg_factory->Header();
    auto sent_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), write_some)
    .WillOnce([this, test_msg, sent_msg](const boost::asio::const_buffers_1 &buffers, boost::system::error_code &ec) {
        const char* src = static_cast<const char*>(buffers.data());
        std::vector<char> buffer(src, src + buffers.size());

        size_t deserilized_bytes = sent_msg->Deserialize(buffer);
        EXPECT_EQ(deserilized_bytes, test_msg->length);

        return sent_msg->length;
    });

    auto bytes_sent = tcp_session->SendMsg(test_msg);

    EXPECT_EQ(bytes_sent, test_msg->length);
    EXPECT_EQ(sent_msg->id, test_msg->id);
    EXPECT_EQ(sent_msg->length, test_msg->length);
    EXPECT_EQ(sent_msg->timestamp, test_msg->timestamp);
}