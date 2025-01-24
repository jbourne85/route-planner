#ifndef ROUTEPLANNER_H
#define ROUTEPLANNER_H

#include <log4cxx/logger.h>
#include "route/ILocationDatabase.h"
#include "route/IRouteDatabase.h"
#include "route/Location.h"

namespace route {

/// @brief this is the main Route planner class, it should know how to create all the routes between locations
class RoutePlanner {
    static log4cxx::LoggerPtr m_logger;
    std::shared_ptr<ILocationDatabase> m_location_db;
    std::shared_ptr<IRouteDatabase> m_route_db;
    
    /// @brief Disable copying of this class
    RoutePlanner(const RoutePlanner& other) {};
protected:
    /// @brief This should use the Location/Route databases to calculate all the destinations from each start location
    /// @return A list of locations with all the end destinations for each location set
    virtual const std::vector<Location*> SetupRoutes() const;

    /// @brief Utility method to get the index of the current min route cost within a cost array, 
    /// ignoring any that have already been processed (as indicated by the shortest path tree set)
    /// @param route_costs Current list of route costs from start to each location 
    /// @param spt_set Current shortest path tree set, indicating whats been proessed and not
    /// @return The index of the minimum route cost in route_costs
    size_t GetMinCostIndex(std::vector<unsigned int>& route_costs, std::vector<bool>& spt_set);

    /// @brief Utility method to covert a location name into a index within a location set
    /// @param locations The list of locations to search
    /// @param location_name The location name to find
    /// @return The index of that location in locations
    size_t GetLocationIndex(const std::vector<Location*>& locations, std::string location_name);
public:

    /// @brief This is the class constructor
    /// @param location_db A unique pointer for the location database
    /// @param route_db A unique pointer for the route database
    RoutePlanner(std::shared_ptr<ILocationDatabase> location_db, std::shared_ptr<IRouteDatabase> route_db);

    /// @brief This should get a list of all the location names avaliable to route between
    /// @return The list of locations avaliable
    std::vector<std::string> GetLocationNames() const;

    /// @brief This gets the cost to travle between two locations
    /// @param start This is the start location
    /// @param destination This is the end location
    /// @return The route cost on success, 0 on failure
    unsigned int GetRouteCost(std::string start_location_name, std::string end_location_name);
};

}

#endif