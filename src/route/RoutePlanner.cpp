#include <iostream>
#include "route/RoutePlanner.h"

namespace route {

log4cxx::LoggerPtr RoutePlanner::m_logger(log4cxx::Logger::getLogger("RoutePlanner"));

RoutePlanner::RoutePlanner(std::shared_ptr<ILocationDatabase> location_db, std::shared_ptr<IRouteDatabase> route_db) :
m_location_db(location_db),
m_route_db(route_db)
{
}

const std::vector<Location*> RoutePlanner::SetupRoutes() const{
    bool locations_updated = m_location_db->Load();
    bool routes_updated = m_route_db->Load();

    if (locations_updated || routes_updated) {
        LOG4CXX_DEBUG(m_logger, "locations/Routes db changed, re-configuring database. locations_updated=" << locations_updated << " routes_updated=" << routes_updated);
        const std::vector<Location*> locations = m_location_db->GetLocations();

        std::for_each(locations.begin(), locations.end(), [this](Location* const start_location) -> void {
            std::vector<std::string> routes = m_route_db->GetRoutes(start_location->Name());
            std::for_each(routes.begin(), routes.end(), [this, start_location](std::string location) -> void {
                const Location* end_location = m_location_db->GetLocation(location);
                if (end_location) {
                    start_location->AddDestination(end_location);
                    LOG4CXX_DEBUG(m_logger, "Configured route Added. " << start_location->Name() << " -> " << end_location->Name());
                }
            });
        });
        LOG4CXX_DEBUG(m_logger, "Configured " << locations.size() << " routes");
        return locations;
    }
    else {
        const std::vector<Location*> locations = m_location_db->GetLocations();
        LOG4CXX_DEBUG(m_logger, "Currently " << locations.size() << " routes configured");
        return locations;
    }
}

std::vector<std::string> RoutePlanner::GetLocationNames() const {
    std::vector<std::string> location_names;

    const std::vector<Location*> locations = SetupRoutes();

    std::for_each(locations.begin(), locations.end(), [this, &location_names](Location* location) -> void {
        location_names.push_back(location->Name());
    });  

    return location_names;
}

unsigned int RoutePlanner::GetRouteCost(std::string start, std::string destination) {
    return 0;
}
}
