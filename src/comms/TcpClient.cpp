#include <functional>
#include <iostream>
#include <string>
#include "comms/TcpClient.h"
#include "comms/TcpSession.h"
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"

namespace comms {

using boost::asio::ip::tcp;
using messages::MsgHeader;

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
        std::cout << "Starting session to " << server_address << ":" << port_num << std::endl;
        m_connected = true;
    }
    else {
        std::cout << "Error establishing conection to " << server_address << ":" << std::to_string(port_num) << '\n';
        return false;
    }

    m_session = std::make_unique<TcpSession<tcp::socket>>(*m_socket);

    return m_connected;
}

void TcpClient::Send(MsgHeader::MsgPointer msg, MsgHeader::MsgHandler msg_handler)
{
    if (m_connected) {
        if(m_session->SendMsg(msg)) 
        {
            auto msg_received = m_session->WaitForMsg();
            auto msg_response = msg_handler(msg_received);

            if (nullptr != msg_response)
            {
                Send(msg_response, msg_handler);
            } 
        }
    }
}

}