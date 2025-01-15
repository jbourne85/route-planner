#include <iostream>
#include "route/RoutePlanner.h"

namespace route {

RoutePlanner::RoutePlanner(std::shared_ptr<ILocationDatabase> location_db, std::shared_ptr<IRouteDatabase> route_db) :
m_location_db(location_db),
m_route_db(route_db)
{
}

std::vector<Location* const> RoutePlanner::GetRoutes() const{
    if (m_location_db->Load() || m_route_db->Load()) {
        std::vector<Location* const> locations = m_location_db->GetLocations();

        std::for_each(locations.begin(), locations.end(), [this](Location* const start_location) -> void {
            std::vector<std::string> routes = m_route_db->GetRoutes(start_location->Name());
            std::for_each(routes.begin(), routes.end(), [this, start_location](std::string location) -> void {
                const Location* end_location = m_location_db->GetLocation(location);
                if (end_location) {
                    start_location->AddDestination(end_location);
                    std::cout << "Route Added. " << start_location->Name() << " -> " << end_location->Name() << std::endl;
                }
            });
        });

        return locations;
    }
    else {
        return m_location_db->GetLocations();
    }
}

std::vector<std::string> RoutePlanner::GetLocations() const {
    std::vector<std::string> location_names;

    std::vector<Location* const> locations = GetRoutes();

    std::for_each(locations.begin(), locations.end(), [this, &location_names](Location* const location) -> void {
        location_names.push_back(location->Name());
    });  

    return location_names;
}

unsigned int RoutePlanner::GetRouteCost(std::string start, std::string destination) {
    return 0;
}
}