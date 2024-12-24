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

    EXPECT_TRUE(nullptr != received_msg);
    EXPECT_TRUE(test_msg->id == received_msg->id);
    EXPECT_TRUE(test_msg->length == received_msg->length);
    EXPECT_TRUE(test_msg->timestamp == received_msg->timestamp);
}