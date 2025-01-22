#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "route/FileLocationDatabase.h"

namespace route {

log4cxx::LoggerPtr FileLocationDatabase::m_logger(log4cxx::Logger::getLogger("FileLocationDatabase"));

FileLocationDatabase::FileLocationDatabase(const std::string location_file) :
m_database_file(location_file),
m_location_map(),
m_location_list() {
}

FileLocationDatabase::~FileLocationDatabase() {
    DeleteLocations(m_location_list);
}

void FileLocationDatabase::DeleteLocations(std::vector<Location*>& locations) {
    std::for_each (m_location_list.begin(), m_location_list.end(), [this](Location* location) -> void {
        delete location;
    });
    locations.clear();
}

void FileLocationDatabase::AddLocation(Location* location) {
    m_location_list.push_back(location);
    m_location_map.insert(std::make_pair(location->Name(), location));
}

std::vector<Location*> FileLocationDatabase::GetLocationsOnDisk() {
    std::vector<Location*> locations_on_disk;
    std::ifstream file(m_database_file);
    
    if (!file.is_open()) {
        LOG4CXX_ERROR(m_logger, "Could not open the database file " << m_database_file);
        return locations_on_disk;
    }

    bool error = false;
    std::string line;

    const unsigned int name_i = 0;
    const unsigned int cost_i = 1;

    while (file.good() && !error) {
        std::getline(file, line);
        std::stringstream ss(boost::trim_copy(line));   
        std::string value;
        std::vector<std::string> location_data; 

        if (ss.str().length()) {
            while (std::getline(ss, value, ',')) { 
                location_data.push_back(boost::trim_copy(value));
            }

            try {
                locations_on_disk.push_back(
                    new Location(
                        location_data[name_i], 
                        std::stoi(location_data[cost_i])
                    )  
                );
                LOG4CXX_DEBUG(m_logger, "Adding location. name=" << locations_on_disk.back()->Name() << " cost=" << locations_on_disk.back()->Cost()); 
            }
            catch (const std::invalid_argument& ex) {
                LOG4CXX_ERROR(m_logger, "Could not parse string to number the database file entry: '" << line << "'");
                LOG4CXX_ERROR(m_logger, "Assuming the database file is malformed");
                DeleteLocations(locations_on_disk);
                error = true;
            }
        }
    }

    LOG4CXX_DEBUG(m_logger, "Loaded " << locations_on_disk.size() << " location(s) config from disk");    

    file.close(); 
    return locations_on_disk;
}


bool FileLocationDatabase::Load() {
    //Get the locations on disk
    auto locations_on_disk = GetLocationsOnDisk();
    if (!locations_on_disk.size()) {
        return false;
    } 

    // Reset the current location database
    DeleteLocations(m_location_list);

    m_location_map.clear();    

    // Re-load with the new locations from disk
    std::for_each (locations_on_disk.begin(), locations_on_disk.end(), [this](Location* location) -> void {
        AddLocation(location);
    });

    return true;
}

const std::vector<Location*> FileLocationDatabase::GetLocations() const {
    return m_location_list;
}

Location* const FileLocationDatabase::GetLocation(std::string location_name) const {
    auto location = m_location_map.find(location_name);
    if (location != m_location_map.end()) {
        return location->second;
    }
    return nullptr;
}

}
