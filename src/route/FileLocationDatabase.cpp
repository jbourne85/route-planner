#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "route/FileLocationDatabase.h"

namespace route {

FileLocationDatabase::FileLocationDatabase(const std::string location_file) :
m_database_file(location_file)
{

}

FileLocationDatabase::~FileLocationDatabase() {
    std::for_each (m_location_list.begin(), m_location_list.end(), [this](const Location* const location) -> void {
        delete location;
    });
}

bool FileLocationDatabase::Load() {

    std::ifstream file(m_database_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the databse file " << m_database_file << std::endl;
        return false;
    }

    // Load the locations from the cvs locations file
    std::string line;
    std::vector<const Location* const> locations_on_disk;

    const unsigned int name_i = 0;
    const unsigned int cost_i = 1;

    while (std::getline(file, line)) { 
        std::stringstream ss(line);   
        std::string value;
        std::vector<std::string> location_data; 
        
        while (std::getline(ss, value, ',')) { 
            location_data.push_back(value);
        }
        locations_on_disk.push_back(
            new Location(
                location_data[name_i], 
                std::stoi(location_data[cost_i])
            )
        );
    }
    file.close(); 

    // Reset the current location database
    std::for_each (m_location_list.begin(), m_location_list.end(), [this](const Location* const location) -> void {
        delete location;
    });
    m_location_list.clear();
    m_location_map.clear();    

    // Re-load with the new locations from disk
    std::for_each (locations_on_disk.begin(), locations_on_disk.end(), [this](const Location* const location) -> void {
        m_location_list.push_back(location);
        m_location_map.insert(std::make_pair(location->Name(), location));
    });

    return true;
}

std::vector<const Location* const> FileLocationDatabase::GetLocations() {
    return m_location_list;
}

const Location* const FileLocationDatabase::GetLocation(std::string location_name) const {
    auto location = m_location_map.find(location_name);
    if (location != m_location_map.end()) {
        return location->second;
    }
    return nullptr;
}
}