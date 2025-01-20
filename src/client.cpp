#include <chrono>
#include <iostream>
#include "comms/TcpClient.h"
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"

using messages::MsgHeader;
using messages::MsgFactory;

MsgHeader::MsgPointer client_handler(MsgHeader::MsgPointer msg)
{
    std::cout << "Client Handler." << msg->Id() <<std::endl;
    
    MsgFactory msg_factory; 
    if (msg->Id() == messages::MSG_STATUS_RESPONSE_ID) {
        std::cout << "Received Status Response Msg. datestring=" << msg->DateString() << " Server is Up!" << std::endl;
        std::cout << "Asking for Locations List." << std::endl;
        return msg_factory.Create(messages::MSG_LOCATIONS_REQUEST_ID);
    }
    else if (msg->Id() == messages::MSG_LOCATIONS_RESPONSE_ID) {
        std::cout << "Received Locations Response Msg." << std::endl;
        
        auto locations_response = MsgHeader::GetDerivedType<messages::MsgLocationsResponse>(msg);
        const std::vector<std::string> locations = locations_response->GetLocations();
        
        std::for_each(locations.begin(), locations.end(), [](std::string location) -> void {
            std::cout << "Location: " << location << std::endl;
        });

        if (locations_response->GetData()->is_paginated) {
            std::cout << "More Data Avaliabe." << std::endl;
        }
    }
    else {
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
            std::shared_ptr<MsgFactory> msg_factory(new MsgFactory);
            auto msg = msg_factory->Create(messages::MSG_STATUS_REQUEST_ID);
            client.Send(msg, MsgHeader::MsgHandler(client_handler), msg_factory);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}