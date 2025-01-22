#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <boost/asio.hpp>
#include <log4cxx/logger.h>
#include "comms/TcpSession.h"
#include "messages/MsgHeader.h"

namespace comms {

/// @brief This top level TcpClient class knows how to connect to and send/receive messages with a TcpServer
class TcpClient {
    static log4cxx::LoggerPtr m_logger;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;
    std::unique_ptr<TcpSession<boost::asio::ip::tcp::socket> > m_session;
    bool m_connected;
public:
    TcpClient();
    
    /// @brief This will start a new client session to connect to a server
    /// @param server_address This is the address of the server to connet to
    /// @param port_num This is the port number of the server to connect to
    /// @return true on success, false on failure
    bool StartSession(std::string server_address, unsigned int port_num);

    /// @brief This will send a message over the established session
    /// @param msg The message to send
    /// @param msg_handler The message handler to call on reception of the response
    void Send(messages::MsgHeader::MsgPointer msg, messages::MsgHeader::MsgHandler msg_handler, messages::MsgFactory::MsgFactoryPtr msg_factory);
};

}

#endif
