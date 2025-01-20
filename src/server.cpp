#include <iostream>
#include <boost/asio.hpp>
#include "comms/TcpServer.h"
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "route/FileLocationDatabase.h"
#include "route/FileRouteDatabase.h"
#include "route/RoutePlanner.h"

using messages::MsgHeader;
using messages::MsgFactory;
using route::FileLocationDatabase;
using route::FileRouteDatabase;
using route::RoutePlanner;

class ServerMsgHandler {
    route::RoutePlanner* m_route_planner;
public:
    ServerMsgHandler(route::RoutePlanner* route_planner) : m_route_planner(route_planner) {}

    MsgHeader::MsgPointer MsgHandler(MsgHeader::MsgPointer msg) {
        MsgFactory msg_factory;
        if (msg->Id() == messages::MSG_STATUS_REQUEST_ID) {   
            std::cout << "Received Status Request Msg. timestamp=" << msg->Timestamp() << std::endl;
            auto response_msg = msg_factory.Create(messages::MSG_STATUS_RESPONSE_ID);
            std::cout << "Sending Status Response Msg." << std::endl;
            return response_msg;
        }
        else if (msg->Id() == messages::MSG_LOCATIONS_REQUEST_ID) {
            std::cout << "Received Locations Request Msg. timestamp=" << msg->Timestamp() << std::endl;
            auto response_msg = MsgHeader::GetDerivedType<messages::MsgLocationsResponse>(msg_factory.Create(messages::MSG_LOCATIONS_RESPONSE_ID));

            auto locations = m_route_planner->GetLocationNames();

            std::for_each(locations.begin(), locations.end(), [&response_msg](std::string location) -> void {
                response_msg->AddLocation(location);
            });

            std::cout << "Sending Locations Response Msg." << std::endl;
            return response_msg;
        }
        else
        {
            std::cout << "Unknown response message" << std::endl;
        }
        return MsgHeader::MsgPointer(nullptr);
    }
};

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cout << "Usage:" << std::endl;
        std::cout << "\tserver [PORT NUMBER] [LOCATION DB FILE] [ROUTE DB FILE]" << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "\tserver 8080 locations.dat routes.dat" << std::endl;
        return 1;
    }

    const unsigned int port_num(std::stoi(argv[1]));
    const std::string locations_db(argv[2]);
    const std::string routes_db(argv[3]);

    std::shared_ptr<FileLocationDatabase> location_db = std::make_shared<FileLocationDatabase>(locations_db);
    std::shared_ptr<FileRouteDatabase> route_db = std::make_shared<FileRouteDatabase>(routes_db);

    RoutePlanner route_planner(location_db, route_db);

    try {
        std::shared_ptr<MsgFactory> msg_factory(new MsgFactory);
        comms::TcpServer server(port_num);

        ServerMsgHandler msg_handler(&route_planner);        

        server.Start(std::bind(&ServerMsgHandler::MsgHandler, &msg_handler, std::placeholders::_1), msg_factory);        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}