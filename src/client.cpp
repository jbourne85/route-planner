#include <chrono>
#include <iostream>
#include <thread>
#include "comms/TcpClient.h"
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"

using messages::MsgHeader;

MsgHeader::MsgPointer client_handler(MsgHeader::MsgPointer msg)
{
    messages::MsgFactory msg_factory; 
    if (msg->id == messages::MSG_STATUS_RESPONSE_ID)
    {
        std::cout << "Received Status Response Msg. timestamp=" << std::ctime(&msg->timestamp) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "Sending Status Request Msg." << std::endl;
        return msg_factory.Create(messages::MSG_STATUS_REQUEST_ID);
    }
    else
    {
        std::cout << "Unknown response message" << std::endl;
    }
    return MsgHeader::MsgPointer(nullptr);
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cout << "Usage:" << std::endl;
        std::cout << "\tclient [HOST ADDRESS] [PORT NUMBER]" << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "\tclient 127.0.0.1 8080" << std::endl;
        return 1;
    }

    const std::string server_address(argv[1]);
    const unsigned int port_num(std::stoi(argv[2]));

    try
    {
        comms::TcpClient client;

        if (client.StartSession(server_address, port_num)) {
            messages::MsgFactory msg_factory; 
            auto msg = msg_factory.Create(messages::MSG_STATUS_REQUEST_ID);
            client.Send(msg, MsgHeader::MsgHandler(client_handler));
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}