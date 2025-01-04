#include <iostream>
#include <boost/asio.hpp>
#include "comms/TcpServer.h"
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"

using messages::MsgHeader;

MsgHeader::MsgPointer server_handler(MsgHeader::MsgPointer msg)
{
    messages::MsgFactory msg_factory;
    if (msg->id == messages::MSG_STATUS_REQUEST_ID)
    {   
        std::cout << "Received Status Request Msg. timestamp=" << msg->timestamp << std::endl;
        auto response_msg = msg_factory.Create(messages::MSG_STATUS_RESPONSE_ID);
        std::cout << "Sending Status Response Msg." << std::endl;
        return response_msg;
    }
    else
    {
        std::cout << "Unknown response message" << std::endl;
    }
    return MsgHeader::MsgPointer(nullptr);
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "Usage:" << std::endl;
        std::cout << "\tserver [PORT NUMBER]" << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "\tserver 8080" << std::endl;
        return 1;
    }

    const unsigned int port_num(std::stoi(argv[1]));

    try
    {
        comms::TcpServer server(port_num);
        server.Start(MsgHeader::MsgHandler(server_handler));        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}