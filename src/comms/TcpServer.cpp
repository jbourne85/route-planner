#include <boost/asio.hpp>
#include <iostream>
#include "comms/TcpServer.h"

namespace comms {

using boost::asio::ip::tcp;
using messages::MsgHeader;
using messages::MsgFactory;

log4cxx::LoggerPtr TcpServer::m_logger(log4cxx::Logger::getLogger("TcpServer"));

TcpServer::TcpServer(unsigned int port) : 
m_io_context(), 
m_acceptor(m_io_context, tcp::endpoint(tcp::v4(), port)), 
m_running(false) {
}

void TcpServer::Start(MsgHeader::MsgHandler msg_handler, MsgFactory::MsgFactoryPtr msg_factory) {
    m_acceptor.async_accept([this, msg_handler, msg_factory](boost::system::error_code err, tcp::socket socket) {
        if (!err) {
            LOG4CXX_INFO(m_logger, "Session started from: " << socket.remote_endpoint().address().to_string() << ":" << socket.remote_endpoint().port());  
            std::make_shared<TcpSession<tcp::socket> >(socket, msg_factory)->AsyncWaitForMsg(msg_handler);

        } else {
            throw std::runtime_error("Server Error:" + err.message());
        }
        Start(msg_handler, msg_factory);
    });

    if (!m_running) {
        LOG4CXX_INFO(m_logger, "Server Listning on port:" << m_acceptor.local_endpoint().port());
        m_running = true;
        m_io_context.run();       
    }
}

}
