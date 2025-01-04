#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "comms/TcpSession.h"
#include "messages/MsgFactory.h"

using namespace comms;
using namespace messages;
using namespace std::placeholders;

class MockTcpSocketExecutor {
public:
};

class MockTcpSocket {
public:
    typedef MockTcpSocketExecutor executor_type;
    TcpMsgMatch::MsgMatchPointer msg_matcher;

    MockTcpSocket() : msg_matcher(new TcpMsgMatch) {

    }
    MockTcpSocket(const MockTcpSocket& other) {}

    MOCK_METHOD(std::size_t, read_some, (const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec));
    MOCK_METHOD(void, async_read_some, (const boost::asio::mutable_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler));
    MOCK_METHOD(std::size_t, write_some, (const boost::asio::const_buffers_1 &buffers, boost::system::error_code &ec));
    MOCK_METHOD(void, async_write_some, (const boost::asio::const_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler));
};

class MockTcpSession : public TcpSession<MockTcpSocket> {
public:
    MockTcpSession(MockTcpSocket& socket) : TcpSession(socket) {}
    MOCK_METHOD(void, ProcessSession, (messages::MsgHeader::MsgPointer received_msg, messages::MsgHeader::MsgHandler msg_handler), (override));
    MOCK_METHOD(void, ProcessSession, (messages::MsgHeader::MsgHandler msg_handler), (override));
};

class TcpSessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_socket = std::make_shared<MockTcpSocket>();
        tcp_session = std::make_shared<MockTcpSession>(*mock_socket);
        msg_factory = std::make_unique<MsgFactory>();
    }

    void TearDown() override {
    }

    size_t SerializeMsg(MsgHeader::MsgPointer test_msg, const boost::asio::mutable_buffers_1& buffers) {
        std::vector<char> test_data;    
        test_msg->Serialize(test_data);

        char* dest = static_cast<char*>(buffers.data());
        std::memcpy(dest, test_data.data(), test_data.size());

        return test_msg->length;
    }

    size_t DeserializeMsg(MsgHeader::MsgPointer test_msg, const boost::asio::const_buffers_1 &buffers) {
        const char* src = static_cast<const char*>(buffers.data());
        std::vector<char> buffer(src, src + buffers.size());
        
        return test_msg->Deserialize(buffer);
    }

    std::shared_ptr<MockTcpSocket> mock_socket;
    std::shared_ptr<MockTcpSession> tcp_session;
    std::unique_ptr<MsgFactory> msg_factory;
};

/// @brief This tests the TcpSession::WaitForMsg where the message is received over a socket in one go
TEST_F(TcpSessionTest, TestWaitForMessage) {  

    auto test_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), read_some).WillOnce([this, test_msg](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec) {
        return SerializeMsg(test_msg, buffers);
    });

    auto received_msg = tcp_session->WaitForMsg();

    EXPECT_NE(nullptr, received_msg);
    EXPECT_EQ(test_msg->id, received_msg->id);
    EXPECT_EQ(test_msg->length, received_msg->length);
    EXPECT_EQ(test_msg->timestamp, received_msg->timestamp);
}

/// @brief This tests the TcpSession::WaitForMsg where an error occurs (a timed_out error) during the wait
TEST_F(TcpSessionTest, TestErrorWhileWaitForMessage) {  

    auto test_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), read_some).WillOnce([this, test_msg](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &err) {
        err = boost::asio::error::timed_out;        
        return 0;
    });

    EXPECT_THROW({
        auto received_msg = tcp_session->WaitForMsg();
    }, std::runtime_error);
}

/// @brief This tests the TcpSession::WaitForMsg where a message is received over a socket in 4 chunks
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

/// @brief This tests the TcpSession::SendMsg where a message is sent over a socket
TEST_F(TcpSessionTest, TestSendMessage) {  

    auto test_msg = msg_factory->Header();
    auto sent_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), write_some)
    .WillOnce([this, test_msg, sent_msg](const boost::asio::const_buffers_1 &buffers, boost::system::error_code &ec) {
        size_t deserilized_bytes = DeserializeMsg(sent_msg, buffers);
        EXPECT_EQ(deserilized_bytes, test_msg->length);

        return sent_msg->length;
    });

    auto bytes_sent = tcp_session->SendMsg(test_msg);

    EXPECT_EQ(bytes_sent, test_msg->length);
    EXPECT_EQ(sent_msg->id, test_msg->id);
    EXPECT_EQ(sent_msg->length, test_msg->length);
    EXPECT_EQ(sent_msg->timestamp, test_msg->timestamp);
}

/// @brief This tests the TcpSession::SendMsg where an error occurs when a message is sent
TEST_F(TcpSessionTest, TestErrorWhileSendMessage) {  

    auto test_msg = msg_factory->Header();
    auto sent_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), write_some)
    .WillOnce([this, test_msg, sent_msg](const boost::asio::const_buffers_1 &buffers, boost::system::error_code &err) {
        err = boost::asio::error::timed_out;        
        return 0;
    });

    EXPECT_THROW({
        tcp_session->SendMsg(test_msg);
    }, std::runtime_error);
}

/// @brief This tests the TcpSession::AsyncWaitForMsg where a message is received in an async manner over a socket
TEST_F(TcpSessionTest, TestAsyncWaitForMessage) {  
    auto test_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), async_read_some).WillOnce([this, test_msg](const boost::asio::mutable_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
        boost::system::error_code err;
        handler(err, SerializeMsg(test_msg, buffers));
    });

    testing::MockFunction<MsgHeader::MsgPointer(const MsgHeader::MsgPointer)> MockMsgHandler;

    EXPECT_CALL(*tcp_session, ProcessSession(testing::_, testing::_))
    .WillOnce([this, test_msg](messages::MsgHeader::MsgPointer received_msg, messages::MsgHeader::MsgHandler msg_handler){
        EXPECT_NE(nullptr, received_msg);
        EXPECT_EQ(test_msg->id, received_msg->id);
        EXPECT_EQ(test_msg->length, received_msg->length);
        EXPECT_EQ(test_msg->timestamp, received_msg->timestamp); 
    });   

    tcp_session->AsyncWaitForMsg(MockMsgHandler.AsStdFunction());
}


/// @brief This tests the TcpSession::AsyncWaitForMsg where an error occurs when a message is received, it is expected that an exception will be thrown
TEST_F(TcpSessionTest, TestErrorWhileAsyncWaitForMessage) {  
    auto test_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), async_read_some).WillOnce([this, test_msg](const boost::asio::mutable_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
        boost::system::error_code err = boost::asio::error::timed_out; 
        handler(err, 0);
    });

    testing::MockFunction<MsgHeader::MsgPointer(const MsgHeader::MsgPointer)> MockMsgHandler;

    EXPECT_CALL(*tcp_session, ProcessSession(testing::_, testing::_)).Times(0);

    EXPECT_THROW({
        tcp_session->AsyncWaitForMsg(MockMsgHandler.AsStdFunction());
    }, std::runtime_error);
}

/// @brief This tests the TcpSession::AsyncSendMsg where a message is sent in an async manner over a socket
TEST_F(TcpSessionTest, TestAsyncSendMessage) {  

    auto test_msg = msg_factory->Header();
    auto sent_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), async_write_some)
    .WillOnce([this, sent_msg](const boost::asio::const_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
        boost::system::error_code err;
        handler(err, DeserializeMsg(sent_msg, buffers));
    });

    EXPECT_CALL(*tcp_session, ProcessSession(testing::_))
    .WillOnce([this, test_msg](messages::MsgHeader::MsgHandler msg_handler){
    });   

    testing::MockFunction<MsgHeader::MsgPointer(const MsgHeader::MsgPointer)> MockMsgHandler;

    tcp_session->AsyncSendMsg(test_msg, MockMsgHandler.AsStdFunction());

    EXPECT_EQ(sent_msg->id, test_msg->id);
    EXPECT_EQ(sent_msg->length, test_msg->length);
    EXPECT_EQ(sent_msg->timestamp, test_msg->timestamp);
}


/// @brief This tests the TcpSession::AsyncSendMsg where an error occurs when a message is sent
TEST_F(TcpSessionTest, TestErrorWhileAsyncSendMessage) {  

    auto test_msg = msg_factory->Header();

    EXPECT_CALL(*(tcp_session->Socket()), async_write_some)
    .WillOnce([this, test_msg](const boost::asio::const_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
        boost::system::error_code err = boost::asio::error::timed_out; 
        handler(err, 0);
    });

    EXPECT_CALL(*tcp_session, ProcessSession(testing::_)).Times(0);

    testing::MockFunction<MsgHeader::MsgPointer(const MsgHeader::MsgPointer)> MockMsgHandler;
    
    EXPECT_THROW({
        tcp_session->AsyncSendMsg(test_msg, MockMsgHandler.AsStdFunction());
    }, std::runtime_error);
}