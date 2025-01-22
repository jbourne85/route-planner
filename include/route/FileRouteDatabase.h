#ifndef ROUTEDATABASE_H
#define ROUTEDATABASE_H

#include <log4cxx/logger.h>
#include <string>
#include <unordered_map>
#include "route/IRouteDatabase.h"

namespace route {

/// @brief This class manages the database of routes. It knows how to load and sync them from a datafile
class FileRouteDatabase : public IRouteDatabase {
    static log4cxx::LoggerPtr m_logger;
    const std::string m_route_file;
    std::unordered_map<std::string, std::vector<std::string>> m_routes; ///A string map of start -> end locations (where end is a list avaliable locations)
protected:
    
    /// @brief This will read the routes currently on disk
    /// @return The list of routes on disk 
    virtual std::unordered_map<std::string, std::vector<std::string>> GetRoutesOnDisk() const;
public:
    FileRouteDatabase(const std::string route_file);    

    /// @brief This will load the routes
    /// @return True on success, False if not
    bool Load();

    /// @brief This will return a list of routes from a given start location
    /// @param start_location_name This is the name of the start location to get the valid routes for 
    /// @return The list of routes from that start location
    std::vector<std::string> GetRoutes(const std::string start_location_name) const;
};

}

#endif