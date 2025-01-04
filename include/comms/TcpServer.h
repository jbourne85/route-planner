#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <boost/asio.hpp>
#include "comms/TcpSession.h"
#include "messages/MsgHeader.h"

namespace comms {
    
/// @brief This top level TcpServer class which knows how to receive connections from a TcpClient to and receive/send messages.
class TcpServer {
    boost::asio::io_context m_io_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    bool m_running;
public:

    /// @brief The class constructor
    /// @param port_num This is the port number to listen on
    TcpServer(unsigned int port);

    /// @brief This will start a new server session to listen fpr connections from a client
    /// @param msg_handler The message handler to call on reception of a message
    void Start(messages::MsgHeader::MsgHandler msg_handler);
};

}

#endif
