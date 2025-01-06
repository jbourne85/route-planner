#ifndef LOCATIONDATABASE_H
#define LOCATIONDATABASE_H

#include <string>
#include <unordered_map>
#include "route/Location.h"
#include "route/ILocationDatabase.h"

namespace route {

/// @brief This class manages the database of locations. It knows how to load and sync it from a datafile
class FileLocationDatabase : public ILocationDatabase {
    const std::string m_database_file;
    std::unordered_map<std::string, const Location* const> m_location_map;  
    std::vector<const Location* const> m_location_list;  
    
    FileLocationDatabase(const FileLocationDatabase& other); //copying of the Location database is dissalowed

protected:
    /// @brief This will delete all the locations in a given location list 
    /// @param locations The list to iterate over and free up the memory
    virtual void DeleteLocations(std::vector<const Location* const>& locations);

    /// @brief This will read the locations currently on disk
    /// @return The list of locations on disk 
    virtual std::vector<const Location* const> GetLocationsOnDisk();

public:
    /// @brief This is the class constructor, taking a string file flor loading the database
    /// @param location_file 
    FileLocationDatabase(const std::string location_file);

    /// @brief This is the class constructor.
    ~FileLocationDatabase();

    /// @brief This should load the location database, repeated calls should refresh the database. 
    /// @return True if the database was updated successfully, False if not
    bool Load();

    /// @brief This returns the current list of locations in the location database
    /// @return List of locations
    std::vector<const Location* const> GetLocations();

    /// @brief This returns a particular location based on its name
    /// @param location_name The name of the location to get
    /// @return A location pointer
    const Location* const GetLocation(std::string location_name) const;  
};

}

#endif