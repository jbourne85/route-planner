#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "route/FileRouteDatabase.h"

namespace route {

FileRouteDatabase::FileRouteDatabase(const std::string route_file) :
m_route_file(route_file)
{
}

std::unordered_map<std::string, std::vector<std::string>> FileRouteDatabase::GetRoutesOnDisk() const {
    std::unordered_map<std::string, std::vector<std::string>> routes_on_disk;
    std::ifstream file(m_route_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the database file " << m_route_file << std::endl;
        return routes_on_disk;
    }

    bool error;
    std::string line;
    
    while (std::getline(file, line) && !error) { 
        std::stringstream ss(line);   
        std::string value;
        std::vector<std::string> route_data; 
        
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

    file.close(); 
    return routes_on_disk;
}


bool FileRouteDatabase::Load() {
    return false;
}

std::vector<std::string> FileRouteDatabase::GetRoute(const std::string start_location) const {
    return std::vector<std::string>();
}


}