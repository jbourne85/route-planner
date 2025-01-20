#include <iostream>
#include <boost/asio.hpp>
#include "comms/TcpServer.h"
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"

using messages::MsgHeader;
using messages::MsgFactory;

MsgHeader::MsgPointer server_handler(MsgHeader::MsgPointer msg)
{
    MsgFactory msg_factory;
    if (msg->Id() == messages::MSG_STATUS_REQUEST_ID)
    {   
        std::cout << "Received Status Request Msg. timestamp=" << msg->Timestamp() << std::endl;
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

    try {
        std::shared_ptr<MsgFactory> msg_factory(new MsgFactory);
        comms::TcpServer server(port_num);
        server.Start(MsgHeader::MsgHandler(server_handler), msg_factory);        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}