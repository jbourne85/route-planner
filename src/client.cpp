#include <chrono>
#include <iostream>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include "comms/TcpClient.h"
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"

using namespace log4cxx;
using namespace messages;


/// @brief This is the main Client Msg handler. It should know how to send request messages to the server and deal with the response messages
class ClientMsgHandler {
    static LoggerPtr m_logger;  /// The class logger
    MsgFactory::MsgFactoryPtr m_msg_factory;    /// The Message Factory to use when creating the requests/responses
    std::vector<std::string> m_locations_cache; /// Local Locations cache as received from the server
public:

    /// @brief class constructor
    /// @param msg_factory This is a msg factory instance
    ClientMsgHandler(MsgFactory::MsgFactoryPtr msg_factory) :
    m_msg_factory(msg_factory) {
    }

    /// @brief This will display the locations in the local locations cache, and ask the user to enter a start and end location
    /// to calculate a route between
    /// @return A Route Request Message
    MsgHeader::MsgPointer GetRouteRequest() {
        std::cout << "The following " << m_locations_cache.size() << " locations are avaliable to route between:" << std::endl;
        size_t index = 0;
        std::for_each(m_locations_cache.begin(), m_locations_cache.end(), [this, &index](std::string location) -> void {
                std::cout << index << " - " << location << std::endl;
                ++index;
        });

        size_t start_location = m_locations_cache.size();
        while(start_location >= m_locations_cache.size()) {
            std::cout << "Please Enter starting location number:" << std::endl;
            std::cin >> start_location;
            std::cin.ignore();
        }

        size_t end_location = m_locations_cache.size();
        while(end_location >= m_locations_cache.size()) {
            std::cout << "Please Enter ending location number:" << std::endl;
            std::cin >> end_location;
            std::cin.ignore();
        }

        std::cout << "Calculating the route cost for: " << m_locations_cache[start_location] << " -> " << m_locations_cache[end_location] << std::endl;

        auto route_request = MsgHeader::GetDerivedType<MsgRouteRequest>(m_msg_factory->Create(MSG_ROUTE_REQUEST_ID));
        route_request->SetStartLocation(start_location);
        route_request->SetEndLocation(end_location);
        
        return route_request;
    }

    /// @brief This method will handle a Status Response Message received from the server, this indicates the server is up so so send
    /// a Locations Request Message
    /// @param status_response This is the Status Response Message to handle
    /// @return A Locations Request Message
    MsgHeader::MsgPointer HandleStatusResponseMsg(MsgStatusResponse::MsgPointer status_response) {
        LOG4CXX_INFO(m_logger, "Received Status Response Msg. datestring=" << status_response->DateString() << " Server is Up!");
        std::cout << "Connected to Route Planner service!" << std::endl;
        
        m_locations_cache.clear();

        return m_msg_factory->Create(MSG_LOCATIONS_REQUEST_ID);
    }

    /// @brief This method will handle a Locations Response Message received from the server, once all the locations have been received from the server
    //  the user will be asked to slect two locations to route between
    /// @param locations_response This is the Locations Response Message to handle
    /// @return A Route Request Message, or Locations Request Message if theres more locations to retreive
    MsgHeader::MsgPointer HandleLocationsResponseMsg(MsgLocationsResponse::MsgPointer locations_response) {
        const std::vector<std::string> locations = locations_response->GetLocations();

        LOG4CXX_INFO(m_logger, "Received Locations Response Msg. timestamp=" << locations_response->DateString() << " locations.n=" << locations.size());       
        
        std::for_each(locations.begin(), locations.end(), [this](std::string location) -> void {
            m_locations_cache.push_back(location);
        });

        if (locations_response->GetData()->is_paginated) {
            LOG4CXX_INFO(m_logger, "More locations are avaliabe, asking for more..");

            auto locations_request = MsgHeader::GetDerivedType<MsgLocationsRequest>(m_msg_factory->Create(MSG_LOCATIONS_REQUEST_ID));
            locations_request->SetStartLocation(m_locations_cache.size());

            return locations_request;
        } 
        else {
            return GetRouteRequest();
        }
        return nullptr;
    }

    /// @brief This method will handle a Route Response Message received from the server and display the result
    /// @param route_response This is the Route Response Message to handle
    /// @return A Locations Request Message, to start the main loop again
    MsgHeader::MsgPointer HandleRouteResponseMsg(MsgRouteResponse::MsgPointer route_response) {
        LOG4CXX_INFO(m_logger, "Received Route Response Msg. timestamp=" << route_response->DateString()); 
        std::cout << "Route Calculation cost " << route_response->GetData()->cost << std::endl;

        m_locations_cache.clear();
        return m_msg_factory->Create(MSG_LOCATIONS_REQUEST_ID);
    }

    /// @brief This is the main Message Handler called on reception of a message
    /// @param msg The message to handle
    /// @return The response message to send
    MsgHeader::MsgPointer MsgHandler(MsgHeader::MsgPointer msg) {
        if (msg->Id() == messages::MSG_STATUS_RESPONSE_ID) {
            return HandleStatusResponseMsg(MsgHeader::GetDerivedType<MsgStatusResponse>(msg));
        }
        else if (msg->Id() == messages::MSG_LOCATIONS_RESPONSE_ID) {
            return HandleLocationsResponseMsg(MsgHeader::GetDerivedType<MsgLocationsResponse>(msg));
        }
        else if (msg->Id() == messages::MSG_ROUTE_RESPONSE_ID) {
            return HandleRouteResponseMsg(MsgHeader::GetDerivedType<MsgRouteResponse>(msg));
        }
        else {
            std::cout << "Unknown response message" << std::endl;
        }
        return nullptr;
    }
};

log4cxx::LoggerPtr ClientMsgHandler::m_logger(log4cxx::Logger::getLogger("ClientMsgHandler"));

int main(int argc, char* argv[])
{
    PropertyConfigurator::configure("log4cxx.properties");
    LoggerPtr logger = Logger::getLogger("server");

    if (argc != 3) {
        std::cout << "Usage:" << std::endl;
        std::cout << "\tclient [HOST ADDRESS] [PORT NUMBER]" << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << "\tclient 127.0.0.1 8080" << std::endl;
        return 1;
    }

    const std::string server_address(argv[1]);
    const unsigned int port_num(std::stoi(argv[2]));

    std::shared_ptr<MsgFactory> msg_factory = std::make_shared<MsgFactory>();
    MsgHeader::MsgPointer msg = msg_factory->Create(messages::MSG_STATUS_REQUEST_ID);

    ClientMsgHandler msg_handler(msg_factory);

    try
    {
        comms::TcpClient client;

        if (client.StartSession(server_address, port_num)) {
            client.Send(msg, std::bind(&ClientMsgHandler::MsgHandler, &msg_handler, std::placeholders::_1), msg_factory);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}