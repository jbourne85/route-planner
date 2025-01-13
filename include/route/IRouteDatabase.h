#ifndef IROUTEDATABASE_H
#define IROUTEDATABASE_H

#include <string>
#include <vector>

namespace route {

/// @brief This should represent a source of routes
class IRouteDatabase {
public:
    virtual ~IRouteDatabase() {}

    /// @brief This should load the route database, repeated calls will reload the database. If an error occurrs on loading
    ///        the previous list of routes will be kept
    /// @return True if the routes were updated successfully, False if not
    virtual bool Load() = 0;

    /// @brief This should returns a particular route based on its name
    /// @param start_location_name The name of the route to get based on the start location name
    /// @return A list of valid routes
    virtual std::vector<std::string> GetRoutes(const std::string start_location_name) const = 0;
};

}

#endif