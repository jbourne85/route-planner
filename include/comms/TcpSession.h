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
    messages::MsgFactory::MsgFactoryPtr m_msg_factory;
    boost::asio::streambuf m_buffer;

    /// @brief This will process the current session based on a received message, it will keep it alive as long as new messages are received
    /// @param received_msg This is the incomming message to process
    /// @param msg_handler This is the message handler to call for a received message, if it returns a new message then this is sent on
    virtual void ProcessSession(messages::MsgHeader::MsgPointer received_msg, messages::MsgHeader::MsgHandler msg_handler) {
        auto response_msg = msg_handler(received_msg);            
        if (response_msg) {
            if (SendMsg(response_msg)) {
                auto msg_received = WaitForMsg();
                ProcessSession(msg_received, msg_handler);
            }
        }
    }

    /// @brief This will process the current session, it waits for a new message before processing
    /// @param msg_handler This is the message handler to call for a received message, if it returns a new message then this is sent on
    virtual void ProcessSession(messages::MsgHeader::MsgHandler msg_handler) {
        auto msg_received = WaitForMsg();
        ProcessSession(msg_received, msg_handler);
    }

public:

    TcpSession(SocketType& socket, messages::MsgFactory::MsgFactoryPtr msg_factory) : m_socket(std::move(socket)), m_msg_factory(msg_factory)
    {}

    virtual ~TcpSession()
    {}

    /// @brief This waits for a valid message to be received on a socket using the boost::asio::async_read_until
    /// @param msg_handler This is the message response handler which will be called on reception of a message
    void AsyncWaitForMsg(messages::MsgHeader::MsgHandler msg_handler) {
        using namespace std::placeholders;
    
        auto self(this->shared_from_this());

        TcpMsgMatch::MsgMatchPointer msg_matcher(new TcpMsgMatch(m_msg_factory));

        boost::asio::async_read_until(m_socket, m_buffer, std::bind(&comms::TcpMsgMatch::ProcessBuffer, msg_matcher, _1, _2), 
        [this, self, msg_handler, msg_matcher](boost::system::error_code err, std::size_t bytes) {
            messages::MsgHeader::MsgPointer received_msg(nullptr);
            if (!err) {
                received_msg = msg_matcher->GetMsg();
            }
            else {
                throw std::runtime_error("Error Receiving Msg: " + err.message());
            }
            self->ProcessSession(received_msg, msg_handler);
        });
    }

    /// @brief This waits for a valid message to be received on a socket
    /// @return A valid MsgHeader::MsgPointer message on success, nullptr on failure
    messages::MsgHeader::MsgPointer WaitForMsg() {
        std::vector<char> buffer;
        const size_t max_buffer_size = m_msg_factory->MaxLength(); 
        char temp_buffer[max_buffer_size];

        size_t total_bytes_read = 0;
        size_t bytes_received;

        boost::system::error_code err;        
        messages::MsgHeader::MsgPointer msg_header = m_msg_factory->Create(messages::MSG_HEADER_ID);

        while (total_bytes_read < msg_header->Length()) {
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
            msg = m_msg_factory->Create(msg_header->Id());
            if(msg) {
                msg->Deserialize(buffer);
            }
        }
        else {
            throw std::runtime_error("Error Receiving Msg: " + err.message());
        }   
        return msg;
    }


    /// @brief This sends a message over a socket using boost::asio::async_write
    /// @param msg This is the message to send
    /// @return the number of bytes sent
    void AsyncSendMsg(messages::MsgHeader::MsgPointer msg, messages::MsgHeader::MsgHandler msg_handler) {
        std::vector<char> buffer;
        msg->Serialize(buffer);

        auto self(this->shared_from_this());

        boost::asio::async_write(m_socket, boost::asio::buffer(buffer), 
        [this, self, msg_handler](boost::system::error_code err, std::size_t bytes_sent) {
            if (!err) {
                self->ProcessSession(msg_handler);
            }
            else {
                throw std::runtime_error("Error Sending Msg: " + err.message());
            }
        });
    }

    /// @brief This sends a message over a socket using boost::asio::write
    /// @param msg This is the message to send
    /// @return the number of bytes sent
    size_t SendMsg(messages::MsgHeader::MsgPointer msg) {
        std::vector<char> buffer;
        msg->Serialize(buffer);
        boost::system::error_code err;
        boost::asio::write(m_socket, boost::asio::buffer(buffer), err);

        size_t bytes_sent = 0;

        if (!err) {
            bytes_sent = msg->Length();
        }
        else {
            throw std::runtime_error("Error Sending Msg: " + err.message());
        }

        return bytes_sent;
    }

    SocketType* Socket() {
        return &m_socket;
    }
};

}

#endif