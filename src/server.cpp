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

/// @brief This is the main Server Msg handler. It should know how to receive request messages from a connected client and respond to them
class ServerMsgHandler {
    static LoggerPtr m_logger;
    MsgFactory::MsgFactoryPtr m_msg_factory;    /// The Message Factory to use when creating the requests/response
    route::RoutePlanner* m_route_planner;   /// The Route planner object, this is what knows how to calculate routes and holds the list of valid routes and locations
public:

    /// @brief class constructor
    /// @param msg_factory This is a msg factory instance
    /// @param route_planner This is a route planner instance
    ServerMsgHandler(MsgFactory::MsgFactoryPtr msg_factory, route::RoutePlanner* route_planner) : 
    m_msg_factory(msg_factory),
    m_route_planner(route_planner)
    {}

    /// @brief This method will handle a Status Request Message received from the client
    /// @param status_request This is the Status Request Message to handle
    /// @return A Status Response Message
    MsgHeader::MsgPointer HandleStatusRequestMsg(MsgStatusRequest::MsgPointer status_request) {
        LOG4CXX_INFO(m_logger, "Received Status Request Msg. timestamp=" << status_request->DateString());
        LOG4CXX_INFO(m_logger, "Sending Status Response Msg.");

        return m_msg_factory->Create(MSG_STATUS_RESPONSE_ID);
    }

    /// @brief This method will handle a Loctions Request Message received from the client. It will handle pagnated requests, especially
    /// if the number of locations is larger than what can be handled via the msg
    /// @param locations_request This is the Loctions Request Message to handle
    /// @return A Loctions Response Message
    MsgHeader::MsgPointer HandleLocationsRequestMsg(MsgLocationsRequest::MsgPointer locations_request) {
        LOG4CXX_INFO(m_logger, "Received Locations Request Msg. timestamp=" << locations_request->DateString());

        auto response_msg = MsgHeader::GetDerivedType<MsgLocationsResponse>(m_msg_factory->Create(MSG_LOCATIONS_RESPONSE_ID));

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

    /// @brief This method will handle a Route Request Message received from the client, it will use the route planner to calculate the route
    /// between a start and end location, sending the result back to the client
    /// @param route_request This is the Route Request Message to handle
    /// @return A Route Response Message
    MsgHeader::MsgPointer HandleRouteRequestMsg(MsgRouteRequest::MsgPointer route_request) {
        LOG4CXX_INFO(m_logger, "Received Route Request Msg. timestamp=" << route_request->DateString());
        size_t start_location = route_request->GetData()->start_location;
        size_t end_location = route_request->GetData()->end_location;

        auto locations = m_route_planner->GetLocationNames();
        if (start_location < locations.size() && end_location < locations.size()) {
            size_t cost = m_route_planner->GetRouteCost(locations[start_location], locations[end_location]);

            auto response_msg = MsgHeader::GetDerivedType<MsgRouteResponse>(m_msg_factory->Create(MSG_ROUTE_RESPONSE_ID));
            response_msg->SetCost(cost);

            return response_msg;
        }
        else {
            LOG4CXX_ERROR(m_logger, "Unexpected start_location / end_location in Route Request msg. start_location=" << start_location << " end_location=" << end_location << " locations.n" << locations.size());
        }
        return nullptr;
    }

    /// @brief This is the main Message Handler called on reception of a message
    /// @param msg The message to handle
    /// @return The response message to send
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
    std::shared_ptr<MsgFactory> msg_factory = std::make_shared<MsgFactory>();

    RoutePlanner route_planner(location_db, route_db);
    ServerMsgHandler msg_handler(msg_factory, &route_planner);    

    try {
        comms::TcpServer server(port_num);
        server.Start(std::bind(&ServerMsgHandler::MsgHandler, &msg_handler, std::placeholders::_1), msg_factory);        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}