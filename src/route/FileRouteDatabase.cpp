#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "route/FileRouteDatabase.h"

namespace route {

log4cxx::LoggerPtr FileRouteDatabase::m_logger(log4cxx::Logger::getLogger("FileRouteDatabase"));

FileRouteDatabase::FileRouteDatabase(const std::string route_file) :
m_route_file(route_file),
m_routes() {
}

std::unordered_map<std::string, std::vector<std::string>> FileRouteDatabase::GetRoutesOnDisk() const {
    std::unordered_map<std::string, std::vector<std::string>> routes_on_disk;
    std::ifstream file(m_route_file);
    if (!file.is_open()) {
        LOG4CXX_ERROR(m_logger, "Could not open the route database file " << m_route_file);
        return routes_on_disk;
    }

    bool error = false;
    std::string line;
    
    while (file.good() && !error) {
        std::getline(file, line);
        std::stringstream ss(boost::trim_copy(line));   
        std::string value;
        std::vector<std::string> route_data; 
        
        if (ss.str().length()) {
            while (std::getline(ss, value, ',')) { 
                route_data.push_back(boost::trim_copy(value));
            }
        
            const std::string source = route_data[0];
            const std::vector<std::string> destinations(route_data.begin() + 1, route_data.end());

            auto route = routes_on_disk.find(source);

            if (route == routes_on_disk.end()) {
                routes_on_disk.insert(std::make_pair(route_data[0], destinations));
            }
            else {
                route->second.insert(route->second.end(), destinations.begin(), destinations.end());
            }  
        }     
    }

    LOG4CXX_DEBUG(m_logger, "Loaded " << routes_on_disk.size() << " route(s) config from disk");    

    file.close(); 
    return routes_on_disk;
}


bool FileRouteDatabase::Load() {
    //Get the routes on disk
    auto routes_on_disk = GetRoutesOnDisk();
    if (!routes_on_disk.size()) {
        return false;
    }

    // Update the routes database
    m_routes = routes_on_disk;    

    return true;
}

std::vector<std::string> FileRouteDatabase::GetRoutes(const std::string start_location) const {
    auto routes = m_routes.find(start_location);
    if (routes != m_routes.end()) {
        return routes->second;
    }
    return std::vector<std::string>();
}

}
