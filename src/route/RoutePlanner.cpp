#include <algorithm>   
#include <iostream>
#include <iterator> 
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

 size_t RoutePlanner::GetMinCostIndex(std::vector<unsigned int>& route_costs, std::vector<bool>& spt_set) {
    int min = INT_MAX, min_index;

	for (size_t i = 0; i < route_costs.size(); ++i) {
		if (spt_set[i] == false && route_costs[i] <= min) {
			min = route_costs[i], min_index = i;
		}
	}

	return min_index;
 }

size_t RoutePlanner::GetLocationIndex(const std::vector<Location*>& locations, std::string location_name) {
    return std::distance(locations.begin(), std::find_if(locations.begin(), locations.end(), [location_name](Location* location) -> bool{
        return location->Name() == location_name;
    }));
}

unsigned int RoutePlanner::GetRouteCost(std::string start_location_name, std::string end_location_name) {
    Location* const start_location = m_location_db->GetLocation(start_location_name);
    Location* const end_location = m_location_db->GetLocation(end_location_name);

    size_t route_cost = 0;

    if (start_location && end_location) {
        const std::vector<Location*> locations = m_location_db->GetLocations();

        LOG4CXX_INFO(m_logger, "Calculating the route cost for: " << start_location->Name() << " -> " << end_location->Name());

        // Use Dijkstra’s Algorithm to find the shortest route coset between start -> end 
        std::vector<unsigned int> route_costs(locations.size(), INT_MAX); // The final route costs which will hold the result of the route from start -> end											                            
        std::vector<bool> spt_set(locations.size(), false);  // shortest path tree list to keep track of which paths have already been calculated

        size_t start_i = GetLocationIndex(locations, start_location_name);
        size_t end_i = GetLocationIndex(locations, end_location_name);

        // Distance of source vertex from itself is always 0
        route_costs[start_i] = 0;

        // Find the minimum costs for all locations, stopping before the end
        for (size_t count = 0; count < locations.size() - 1; ++count) {
            int min_cost_i = GetMinCostIndex(route_costs, spt_set);
            spt_set[min_cost_i] = true;
            const Location* const ref_location = locations[min_cost_i];
            unsigned int base_cost = route_costs[min_cost_i];
            bool bSearch = true;

            // Update the distance cost for all the adjacent locations
            for (size_t adj_i = 0; adj_i < locations.size() && bSearch; ++adj_i) {
                size_t adjacent_location = adj_i;

                //Check the adjacent locations of the current reference location
                if (ref_location->DestinationIsValid(locations[adjacent_location])) {
                    
                    unsigned int dest_cost = locations[adjacent_location]->Cost();
                    
                    //Update the route cost of the current adjacent location and the total route cost from reference
                    //location through this is less than the current cost
                    if (!spt_set[adj_i] && base_cost != INT_MAX && base_cost + dest_cost < route_costs[adj_i]) {
                        route_costs[adj_i] = base_cost + dest_cost;

                        if(adj_i == end_i) {
                            bSearch = false;   
                        }
                    }
                }
            }
        }
        route_cost = route_costs[end_i] + locations[start_i]->Cost();
        LOG4CXX_INFO(m_logger, "Calculated the route cost: " << start_location->Name() << " -> " << end_location->Name() << " cost=" << route_cost);
    }
   
    return route_cost;
}
}
