#ifndef LOCATION_H
#define LOCATION_H

#include <string>
#include <vector>

namespace route {

/// @brief This represenst a single location in the graph, and knows valid routes to other locations
class Location {
    const std::string m_name;                       /// This is the string representation of the Location
    unsigned int m_cost;                            /// This is the point cost assoctaied with this point
    std::vector<const Location* const> m_routes;    /// This is a list of locations that are reachable from this location
public:
    Location(const std::string name, unsigned int cost);

    /// @brief Getter for the string location name
    /// @return String representation of the location
    const std::string Name() const;

    /// @brief Getter for the point cost of the location
    /// @return Int representing the point cost
    const unsigned int Cost() const; 

    /// @brief Getter for the valid routes from this location
    /// @return List of valid routes
    std::vector<const Location* const> Routes() const; 
};

}

#endif