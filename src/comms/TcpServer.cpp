#include <boost/asio.hpp>
#include <iostream>
#include "comms/TcpServer.h"
#include "comms/TcpSession.h"
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

namespace comms {

using boost::asio::ip::tcp;
using messages::MsgHeader;

TcpServer::TcpServer(unsigned int port) : m_io_context(), m_acceptor(m_io_context, tcp::endpoint(tcp::v4(), port)), m_running(false)
{
    std::cout << "Starting Server on port:" << port << std::endl;
}

void TcpServer::Start(MsgHeader::MsgHandler msg_handler) {
    m_acceptor.async_accept([this, msg_handler](boost::system::error_code err, tcp::socket socket) {
        if (!err) {
            std::cout << "Session started from: " << socket.remote_endpoint().address().to_string() << ":" << socket.remote_endpoint().port() << '\n';
        
            std::make_shared<TcpSession<tcp::socket> >(socket)->AsyncWaitForMsg(msg_handler);

        } else {
            throw std::runtime_error("Server Error:" + err.message());
        }
        Start(msg_handler);
    });

    if (!m_running) {
        std::cout << "Listning on port " << m_acceptor.local_endpoint().port() << "!" << std::endl;
        m_running = true;
        m_io_context.run();       
    }
}

}
