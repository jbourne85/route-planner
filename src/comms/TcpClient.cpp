#include <iostream>
#include "comms/TcpClient.h"

namespace comms {

using boost::asio::ip::tcp;
using messages::MsgHeader;
using messages::MsgFactory;

log4cxx::LoggerPtr TcpClient::m_logger(log4cxx::Logger::getLogger("TcpClient"));

TcpClient::TcpClient() : 
m_socket(nullptr),
m_session(nullptr),
m_connected(false) {      
}

bool TcpClient::StartSession(std::string server_address, unsigned int port_num) {

    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);

    m_socket = std::make_unique<tcp::socket>(io_context);

    tcp::resolver::results_type endpoints = resolver.resolve(server_address, std::to_string(port_num));

    boost::system::error_code err;
    boost::asio::connect(*m_socket, endpoints, err);     

    if (!err) {
        LOG4CXX_INFO(m_logger, "Starting session to " << server_address << ":" << port_num);
        m_connected = true;
    }
    else {
        LOG4CXX_ERROR(m_logger, "Error establishing conection to " << server_address << ":" << std::to_string(port_num));
        return false;
    }

    return m_connected;
}

void TcpClient::Send(MsgHeader::MsgPointer msg, MsgHeader::MsgHandler msg_handler, MsgFactory::MsgFactoryPtr msg_factory)
{
    if (m_connected) {
        if (!m_session) {
            m_session = std::make_unique<TcpSession<tcp::socket>>(*m_socket, msg_factory);
        }

        LOG4CXX_INFO(m_logger, "Sending Msg. id=" << msg->Id());
        if(m_session->SendMsg(msg)) {
            auto msg_received = m_session->WaitForMsg();
            auto msg_response = msg_handler(msg_received);

            if (msg_response) {
                Send(msg_response, msg_handler, msg_factory);
            } 
            else {
                LOG4CXX_INFO(m_logger, "Session ended, no further messages");
            }
        }
        else {
            LOG4CXX_ERROR(m_logger, "Failed to send Msg. id=" << msg->Id());
        }
    }
}

}