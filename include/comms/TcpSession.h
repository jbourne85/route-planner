#ifndef TCPSESSION_H
#define TCPSESSION_H

#include <boost/asio.hpp>
#include <functional>
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "comms/TcpMsgMatch.h"

namespace comms {

/// @brief This class knows how to send/receive messages over a socket. It is expected that the socket will be of a type 
///        boost::asio::ip::tcp::socket.
template<typename SocketType>
class TcpSession : public std::enable_shared_from_this<TcpSession<SocketType> > {
protected:
    SocketType m_socket;
    messages::MsgFactory m_msg_factory;
    boost::asio::streambuf m_buffer;

public:

    TcpSession(SocketType& socket) : m_socket(std::move(socket))
    {}

    ~TcpSession()
    {}

    /// @brief This waits for a valid message to be received on a socket using the boost::asio::async_read_until
    /// @param msg_response_handler This is the message response handler which will be called on reception of a message
    void AsyncWaitForMsg(messages::MsgHeader::MsgHandler msg_response_handler) {
        using namespace std::placeholders;

        TcpMsgMatch::MsgMatchPointer msg_matcher(new TcpMsgMatch);

        boost::asio::async_read_until(m_socket, m_buffer, std::bind(&comms::TcpMsgMatch::ProcessBuffer, msg_matcher, _1, _2), 
        [this, msg_response_handler, msg_matcher](boost::system::error_code err, std::size_t) {
            messages::MsgHeader::MsgPointer received_msg(nullptr);
            if (!err) {
                received_msg = msg_matcher->GetMsg();
            }
            else {
                std::cout << "Error Receiving Msg: " << err.message() << std::endl; 
            }

            auto response_msg = msg_response_handler(received_msg);            
            if (response_msg) {
                AsyncSendMsg(response_msg);
            }
        });
    }

    /// @brief This waits for a valid message to be received on a socket
    /// @return A valid MsgHeader::MsgPointer message on success, nullptr on failure
    messages::MsgHeader::MsgPointer WaitForMsg() {
        
        std::vector<char> buffer;
        const size_t max_buffer_size = m_msg_factory.MaxLength(); 
        char temp_buffer[max_buffer_size];

        size_t total_bytes_read = 0;
        size_t bytes_received;

        boost::system::error_code err;

        messages::MsgHeader::MsgPointer msg_header = m_msg_factory.Create(messages::MSG_HEADER_ID);

        while (total_bytes_read < msg_header->length) {
            bytes_received = m_socket.read_some(boost::asio::buffer(temp_buffer, max_buffer_size), err);           
            total_bytes_read += bytes_received;        

            if (err) {
                break;
            }

            buffer.insert(buffer.end(), temp_buffer, temp_buffer + bytes_received);
            msg_header->Deserialize(buffer);
        } 

        messages::MsgHeader::MsgPointer msg = messages::MsgHeader::MsgPointer(nullptr);

        if (!err) {
            msg = m_msg_factory.Create(msg_header->id);
            if(msg) {
                msg->Deserialize(buffer);
            }
        }
        else {
            std::cout << "Error Receiving data" << std::endl; 
        }   
        return msg;
    }

    void AsyncSendMsg(messages::MsgHeader::MsgPointer msg) {

    }

    /// @brief This sends a message to be over a socket using boost::asio::write
    /// @param msg This is the message to send
    /// @return the number of bytes sent
    size_t SendMsg(messages::MsgHeader::MsgPointer msg) {
        std::vector<char> buffer;
        msg->Serialize(buffer);
        boost::system::error_code err;
        boost::asio::write(m_socket, boost::asio::buffer(buffer), err);

        size_t bytes_sent = 0;

        if (!err) {
            bytes_sent = msg->length;
        }
        else {
            std::cout << "Error Sending data" << std::endl; 
        }
        return bytes_sent;
    }

    SocketType* Socket() {
        return &m_socket;
    }
};

}

#endif