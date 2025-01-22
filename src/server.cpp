#include <iostream>
#include <boost/asio.hpp>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include "comms/TcpServer.h"
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "route/FileLocationDatabase.h"
#include "route/FileRouteDatabase.h"
#include "route/RoutePlanner.h"

using namespace log4cxx;
using namespace messages;
using namespace route;

class ServerMsgHandler {
    static LoggerPtr m_logger;
    route::RoutePlanner* m_route_planner;
    MsgFactory m_msg_factory;
public:
    ServerMsgHandler(route::RoutePlanner* route_planner) : 
    m_route_planner(route_planner)
    {}

    MsgHeader::MsgPointer HandleStatusRequestMsg(MsgStatusRequest::MsgPointer status_request) {
        LOG4CXX_INFO(m_logger, "Received Status Request Msg. timestamp=" << status_request->DateString());
        LOG4CXX_INFO(m_logger, "Sending Status Response Msg.");

        return m_msg_factory.Create(MSG_STATUS_RESPONSE_ID);
    }

    MsgHeader::MsgPointer HandleLocationsRequestMsg(MsgLocationsRequest::MsgPointer locations_request) {
        LOG4CXX_INFO(m_logger, "Received Locations Request Msg. timestamp=" << locations_request->DateString());

        auto response_msg = MsgHeader::GetDerivedType<MsgLocationsResponse>(m_msg_factory.Create(MSG_LOCATIONS_RESPONSE_ID));

        auto locations = m_route_planner->GetLocationNames();
        size_t start_i = locations_request->GetData()->start_location;

        if (start_i < locations.size()) {
            std::for_each(locations.begin() + start_i, locations.end(), [&response_msg](std::string location) -> void {
                response_msg->AddLocation(location);
            });

            LOG4CXX_INFO(m_logger, "Sending Locations Response Msg.");
            return response_msg;
        }
        else {
            LOG4CXX_ERROR(m_logger, "Unexpected start_location in Locations Request msg. start_location=" << start_i);
        }
        return nullptr;
    }

    MsgHeader::MsgPointer HandleRouteRequestMsg(MsgRouteRequest::MsgPointer route_request) {
        LOG4CXX_INFO(m_logger, "Received Route Request Msg. timestamp=" << route_request->DateString());
        size_t start_location = route_request->GetData()->start_location;
        size_t end_location = route_request->GetData()->end_location;

        auto locations = m_route_planner->GetLocationNames();
        if (start_location < locations.size() && end_location < locations.size()) {
            size_t cost = m_route_planner->GetRouteCost(locations[start_location], locations[end_location]);

            auto response_msg = MsgHeader::GetDerivedType<MsgRouteResponse>(m_msg_factory.Create(MSG_ROUTE_RESPONSE_ID));
            response_msg->SetCost(cost);

            return response_msg;
        }
        else {
            LOG4CXX_ERROR(m_logger, "Unexpected start_location / end_location in Route Request msg. start_location=" << start_location << " end_location=" << end_location << " locations.n" << locations.size());
        }
        return nullptr;
    }

    MsgHeader::MsgPointer MsgHandler(MsgHeader::MsgPointer msg) {
        
        if (msg->Id() == MSG_STATUS_REQUEST_ID) {   
            return HandleStatusRequestMsg(MsgHeader::GetDerivedType<MsgStatusRequest>(msg));
        }
        else if (msg->Id() == MSG_LOCATIONS_REQUEST_ID) {
            return HandleLocationsRequestMsg(MsgHeader::GetDerivedType<MsgLocationsRequest>(msg));
        }
        else if (msg->Id() == MSG_ROUTE_REQUEST_ID) {
            return HandleRouteRequestMsg(MsgHeader::GetDerivedType<MsgRouteRequest>(msg));
        }
        else {
            LOG4CXX_WARN(m_logger, "Unknown response Msg. id=" << msg->Id());
        }
        return nullptr;
    }
};

log4cxx::LoggerPtr ServerMsgHandler::m_logger(log4cxx::Logger::getLogger("ServerMsgHandler"));

int main(int argc, char* argv[])
{
    PropertyConfigurator::configure("log4cxx.properties");
    LoggerPtr logger = Logger::getLogger("server");

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