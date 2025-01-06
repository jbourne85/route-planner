#ifndef LOCATION_H
#define LOCATION_H

#include <string>
#include <unordered_map>

namespace route {

/// @brief This represenst a single location in the graph, and knows valid destinations from this locations
class Location {
    const std::string m_name;                                               /// This is the string representation of the Location
    unsigned int m_cost;                                                    /// This is the point cost assoctaied with this point
    std::unordered_map<std::string, const Location* const> m_destinations;  /// This is a list of destinations that are reachable from this location
public:
    typedef std::unordered_map<std::string, const Location* const> ValidDestinationsType;

    Location(const std::string name, unsigned int cost);

    /// @brief Getter for the string location name
    /// @return String representation of the location
    const std::string Name() const;

    /// @brief Getter for the point cost of the location
    /// @return Int representing the point cost
    const unsigned int Cost() const; 

    /// @brief Getter for the valid destinations from this location
    /// @return Map of valid destinations
    ValidDestinationsType Destinations() const; 

    /// @brief Setter for adding a valid destination from this location
    /// @param location location to add as a valid destination from this
    void AddDestination(const Location* const location);
};

}

#endif