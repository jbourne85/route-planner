#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
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

    MockTcpSocket() {
    }

    MockTcpSocket(const MockTcpSocket& other) {}

    MOCK_METHOD(std::size_t, read_some, (const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec));
    MOCK_METHOD(void, async_read_some, (const boost::asio::mutable_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler));
    MOCK_METHOD(std::size_t, write_some, (const boost::asio::const_buffers_1 &buffers, boost::system::error_code &ec));
    MOCK_METHOD(void, async_write_some, (const boost::asio::const_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler));
};

class MockTcpSession : public TcpSession<MockTcpSocket> {
public:
    MockTcpSession(MockTcpSocket& socket, MsgFactory::MsgFactoryPtr msg_factory) : TcpSession(socket, msg_factory) {}
    MOCK_METHOD(void, ProcessSession, (messages::MsgHeader::MsgPointer received_msg, messages::MsgHeader::MsgHandler msg_handler), (override));
    MOCK_METHOD(void, ProcessSession, (messages::MsgHeader::MsgHandler msg_handler), (override));
};

const unsigned int MOCK_TCP_MSG_ID = 999;

struct MockTcpSessionMsgData {
    char padding[1024];

    MockTcpSessionMsgData() {
        memset(padding, ' ', 1024);
    }
};

class MockTcpSessionMsg : public MsgHeader {
 public:  
    MockTcpSessionMsgData mock_body;
    MockTcpSessionMsg() : MsgHeader(MOCK_TCP_MSG_ID, sizeof(MockTcpSessionMsgData), (char* const)&mock_body) {}
};

class MockTcpMessageFactory : public MsgFactory {
public:
    MOCK_METHOD(MsgHeader::MsgPointer, Create, (const unsigned int id), (override, const) );
    MOCK_METHOD(size_t, MaxLength, (), (override, const));
};

class TcpSessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        log4cxx::PropertyConfigurator::configure("log4cxx.properties");

        msg_factory = std::make_shared<MockTcpMessageFactory>();
        mock_socket = std::make_shared<MockTcpSocket>();      

        //Needed for the msg matcher, really should mock that out so this isn't required
        ON_CALL(*msg_factory, Create(testing::_))
        .WillByDefault([](const unsigned int id) {
            EXPECT_EQ(messages::MSG_HEADER_ID, id);
            MsgFactory factory;
            return factory.Header();
        });

        tcp_session = std::make_shared<MockTcpSession>(*mock_socket, msg_factory);  
    }

    void TearDown() override {
    }

    size_t SerializeMsg(std::vector<char> char_buffer, size_t offset, const boost::asio::mutable_buffers_1& buffers) {
        char* dest = static_cast<char*>(buffers.data());
        std::memcpy(dest, char_buffer.data() + offset, buffers.size());
        return buffers.size();
    }

    size_t SerializeMsg(MsgHeader::MsgPointer test_msg, const boost::asio::mutable_buffers_1& buffers) {
        std::vector<char> test_data;    
        test_msg->Serialize(test_data);

        return SerializeMsg(test_data, 0, buffers);
    }

    size_t DeserializeMsg(MsgHeader::MsgPointer test_msg, const boost::asio::const_buffers_1 &buffers) {
        const char* src = static_cast<const char*>(buffers.data());
        std::vector<char> buffer(src, src + buffers.size());
        return test_msg->Deserialize(buffer);
    }

    std::shared_ptr<MockTcpSocket> mock_socket;
    std::shared_ptr<MockTcpSession> tcp_session;
    std::shared_ptr<MockTcpMessageFactory> msg_factory;
};

/// @brief This tests the TcpSession::WaitForMsg where the message is received over a socket in one go
TEST_F(TcpSessionTest, TestWaitForMessage) {  
    
    auto mock_msg = std::make_shared<MockTcpSessionMsg>();

    EXPECT_CALL(*msg_factory, Create(testing::_))
    .Times(2)
    .WillOnce([](const unsigned int id) {
        EXPECT_EQ(messages::MSG_HEADER_ID, id);
        MsgFactory factory;
        return factory.Header();
    })
    .WillOnce([](const unsigned int id) {
        EXPECT_EQ(MOCK_TCP_MSG_ID, id);
        return std::make_shared<MockTcpSessionMsg>();
    });

    EXPECT_CALL(*msg_factory, MaxLength()).WillOnce(testing::Return(mock_msg->Length()));

    EXPECT_CALL(*(tcp_session->Socket()), read_some).WillOnce([this, mock_msg](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec) {
        return SerializeMsg(mock_msg, buffers);
    });

    auto received_msg = tcp_session->WaitForMsg();
    
    EXPECT_NE(nullptr, received_msg);
    EXPECT_EQ(mock_msg->Id(), received_msg->Id());
    EXPECT_EQ(mock_msg->Length(), received_msg->Length());
    EXPECT_EQ(mock_msg->Timestamp(), received_msg->Timestamp());
}

/// @brief This tests the TcpSession::WaitForMsg where an error occurs (a timed_out error) during the wait
TEST_F(TcpSessionTest, TestErrorWhileWaitForMessage) {  

    auto mock_msg = std::make_shared<MockTcpSessionMsg>();

    EXPECT_CALL(*msg_factory, Create(testing::_))
    .Times(1)
    .WillOnce([](const unsigned int id) {
        EXPECT_EQ(messages::MSG_HEADER_ID, id);
        MsgFactory factory;
        return factory.Header();
    });

    EXPECT_CALL(*msg_factory, MaxLength()).WillOnce(testing::Return(mock_msg->Length()));

    EXPECT_CALL(*(tcp_session->Socket()), read_some).WillOnce([](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &err) {
        err = boost::asio::error::timed_out;        
        return 0;
    });

    EXPECT_THROW({
        auto received_msg = tcp_session->WaitForMsg();
    }, std::runtime_error);
}

/// @brief This tests the TcpSession::WaitForMsg where a message is received over a socket in 4 chunks
TEST_F(TcpSessionTest, TestWaitForMessageInChunks) {  

    auto mock_msg = std::make_shared<MockTcpSessionMsg>();
    std::vector<char> test_data;    
    mock_msg->Serialize(test_data);

    //test the header being read in chunks of 4 
    EXPECT_CALL(*msg_factory, Create(testing::_))
    .Times(2)
    .WillOnce([](const unsigned int id) {
        EXPECT_EQ(messages::MSG_HEADER_ID, id);
        MsgFactory factory;
        return factory.Header();
    })
    .WillOnce([mock_msg](const unsigned int id) {
        EXPECT_EQ(MOCK_TCP_MSG_ID, id);
        return mock_msg;
    });

    EXPECT_CALL(*msg_factory, MaxLength()).WillOnce(testing::Return(mock_msg->Length()));

    EXPECT_FALSE(mock_msg->Length() % 4);         //check the header can be split into 4 chunks for the test
    size_t chunk_size = mock_msg->Length() / 4;
    size_t chunk_i = 1;

    EXPECT_CALL(*(tcp_session->Socket()), read_some)
    .Times(4)  
    .WillRepeatedly([this, test_data, chunk_size, &chunk_i](const boost::asio::mutable_buffers_1 &buffers, boost::system::error_code &ec) {
        char* dest = static_cast<char*>(buffers.data());
        std::memcpy(dest, test_data.data(), chunk_size * chunk_i);
        chunk_i++;
        return chunk_size;
    });

    auto received_msg = tcp_session->WaitForMsg();

    EXPECT_NE(nullptr, received_msg);
    EXPECT_EQ(mock_msg->Id(), received_msg->Id());
    EXPECT_EQ(mock_msg->Length(), received_msg->Length());
    EXPECT_EQ(mock_msg->Timestamp(), received_msg->Timestamp());
}

/// @brief This tests the TcpSession::SendMsg where a message is sent over a socket
TEST_F(TcpSessionTest, TestSendMessage) {  

    auto mock_msg = std::make_shared<MockTcpSessionMsg>();
    auto sent_msg = std::make_shared<MockTcpSessionMsg>();

    EXPECT_CALL(*(tcp_session->Socket()), write_some)
    .WillOnce([this, mock_msg, sent_msg](const boost::asio::const_buffers_1 &buffers, boost::system::error_code &ec) {
        size_t deserilized_bytes = DeserializeMsg(sent_msg, buffers);
        EXPECT_EQ(deserilized_bytes, mock_msg->Length());

        return sent_msg->Length();
    });

    auto bytes_sent = tcp_session->SendMsg(mock_msg);

    EXPECT_EQ(bytes_sent, mock_msg->Length());
    EXPECT_EQ(sent_msg->Id(), mock_msg->Id());
    EXPECT_EQ(sent_msg->Length(), mock_msg->Length());
    EXPECT_EQ(sent_msg->Timestamp(), mock_msg->Timestamp());
}

/// @brief This tests the TcpSession::SendMsg where an error occurs when a message is sent
TEST_F(TcpSessionTest, TestErrorWhileSendMessage) {  

    auto mock_msg = std::make_shared<MockTcpSessionMsg>();

    EXPECT_CALL(*(tcp_session->Socket()), write_some)
    .WillOnce([](const boost::asio::const_buffers_1 &buffers, boost::system::error_code &err) {
        err = boost::asio::error::timed_out;        
        return 0;
    });

    EXPECT_THROW({
        tcp_session->SendMsg(mock_msg);
    }, std::runtime_error);
}

/// @brief This tests the TcpSession::AsyncWaitForMsg where a message is received in an async manner over a socket
TEST_F(TcpSessionTest, TestAsyncWaitForMessage) {  

    auto mock_msg = std::make_shared<MockTcpSessionMsg>();
    std::vector<char> mock_msg_buffer;    
    size_t mock_msg_offset = 0;
    mock_msg->Serialize(mock_msg_buffer);

    EXPECT_CALL(*msg_factory, Create(testing::_))
    .Times(4)
    .WillOnce([](const unsigned int id) {
        EXPECT_EQ(messages::MSG_HEADER_ID, id);
        MsgFactory factory;
        return factory.Header();
    })
    .WillRepeatedly([mock_msg](const unsigned int id) {
        EXPECT_EQ(MOCK_TCP_MSG_ID, id);
        return mock_msg;
    });
    
    EXPECT_CALL(*(tcp_session->Socket()), async_read_some).Times(3)
    .WillRepeatedly([this, &mock_msg_buffer, &mock_msg_offset](const boost::asio::mutable_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
        boost::system::error_code err;

        mock_msg_offset += SerializeMsg(mock_msg_buffer, mock_msg_offset, buffers);
        handler(err, mock_msg_offset);
    });

    testing::MockFunction<MsgHeader::MsgPointer(const MsgHeader::MsgPointer)> MockMsgHandler;

    EXPECT_CALL(*tcp_session, ProcessSession(testing::_, testing::_))
    .WillOnce([this, mock_msg](messages::MsgHeader::MsgPointer received_msg, messages::MsgHeader::MsgHandler msg_handler){
        EXPECT_NE(nullptr, received_msg);
        EXPECT_EQ(mock_msg->Id(), received_msg->Id());
        EXPECT_EQ(mock_msg->Length(), received_msg->Length());
        EXPECT_EQ(mock_msg->Timestamp(), received_msg->Timestamp()); 
    });   

    tcp_session->AsyncWaitForMsg(MockMsgHandler.AsStdFunction());
}


// /// @brief This tests the TcpSession::AsyncWaitForMsg where an error occurs when a message is received, it is expected that an exception will be thrown
TEST_F(TcpSessionTest, TestErrorWhileAsyncWaitForMessage) {  
    
    EXPECT_CALL(*msg_factory, Create(testing::_))
    .Times(1)
    .WillOnce([](const unsigned int id) {
        EXPECT_EQ(messages::MSG_HEADER_ID, id);
        MsgFactory factory;
        return factory.Header();
    });

    EXPECT_CALL(*(tcp_session->Socket()), async_read_some).WillOnce([this](const boost::asio::mutable_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
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

    auto mock_msg = std::make_shared<MockTcpSessionMsg>();
    auto sent_msg = std::make_shared<MockTcpSessionMsg>();

    EXPECT_CALL(*(tcp_session->Socket()), async_write_some)
    .WillOnce([this, sent_msg](const boost::asio::const_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
        boost::system::error_code err;
        handler(err, DeserializeMsg(sent_msg, buffers));
    });

    EXPECT_CALL(*tcp_session, ProcessSession(testing::_))
    .WillOnce([this, mock_msg](messages::MsgHeader::MsgHandler msg_handler){
    });   

    testing::MockFunction<MsgHeader::MsgPointer(const MsgHeader::MsgPointer)> MockMsgHandler;

    tcp_session->AsyncSendMsg(mock_msg, MockMsgHandler.AsStdFunction());

    EXPECT_EQ(sent_msg->Id(), mock_msg->Id());
    EXPECT_EQ(sent_msg->Length(), mock_msg->Length());
    EXPECT_EQ(sent_msg->Timestamp(), mock_msg->Timestamp());
}


// /// @brief This tests the TcpSession::AsyncSendMsg where an error occurs when a message is sent
TEST_F(TcpSessionTest, TestErrorWhileAsyncSendMessage) {  

    auto mock_msg = std::make_shared<MockTcpSessionMsg>();

    EXPECT_CALL(*(tcp_session->Socket()), async_write_some)
    .WillOnce([this](const boost::asio::const_buffers_1& buffers, std::function<void(const boost::system::error_code&, std::size_t)> handler) {
        boost::system::error_code err = boost::asio::error::timed_out; 
        handler(err, 0);
    });

    EXPECT_CALL(*tcp_session, ProcessSession(testing::_)).Times(0);

    testing::MockFunction<MsgHeader::MsgPointer(const MsgHeader::MsgPointer)> MockMsgHandler;
    
    EXPECT_THROW({
        tcp_session->AsyncSendMsg(mock_msg, MockMsgHandler.AsStdFunction());
    }, std::runtime_error);
}