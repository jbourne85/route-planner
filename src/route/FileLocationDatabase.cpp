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
    DeleteLocations(m_location_list);
}

void FileLocationDatabase::DeleteLocations(std::vector<const Location* const>& locations) {
    std::for_each (m_location_list.begin(), m_location_list.end(), [this](const Location* const location) -> void {
        delete location;
    });
    locations.clear();
}

void FileLocationDatabase::AddLocation(const Location* const location) {
    m_location_list.push_back(location);
    m_location_map.insert(std::make_pair(location->Name(), location));
}

std::vector<const Location* const> FileLocationDatabase::GetLocationsOnDisk() {
    std::vector<const Location* const> locations_on_disk;
    std::ifstream file(m_database_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the database file " << m_database_file << std::endl;
        return locations_on_disk;
    }

    bool error;
    std::string line;

    const unsigned int name_i = 0;
    const unsigned int cost_i = 1;
    
    while (std::getline(file, line) && !error) { 
        std::stringstream ss(line);   
        std::string value;
        std::vector<std::string> location_data; 
        
        while (std::getline(ss, value, ',')) { 
            location_data.push_back(value);
        }

        try {
            locations_on_disk.push_back(
                new Location(
                    location_data[name_i], 
                    std::stoi(location_data[cost_i])
                )
            );
        }
        catch (const std::invalid_argument& ex) {
            std::cerr << "Error: Could not parse string to number the database file entry: '" << line << "'" << std::endl;
            std::cerr << "Error: Assuming the database file is malformed" << std::endl;
            DeleteLocations(locations_on_disk);
            error = true;
        }
    }

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
    std::for_each (locations_on_disk.begin(), locations_on_disk.end(), [this](const Location* const location) -> void {
        AddLocation(location);
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