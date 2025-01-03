#ifndef TCPSESSION_H
#define TCPSESSION_H

#include <boost/asio.hpp>
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

public:

    TcpSession(SocketType& socket) : m_socket(std::move(socket))
    {}

    ~TcpSession()
    {}

    void AsyncWaitForMsg(messages::MsgHeader::MsgHandler msg_response_handler) {

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