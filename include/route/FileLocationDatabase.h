#ifndef LOCATIONDATABASE_H
#define LOCATIONDATABASE_H

#include <log4cxx/logger.h>
#include <string>
#include <unordered_map>
#include "route/Location.h"
#include "route/ILocationDatabase.h"

namespace route {

/// @brief This class manages the database of locations. It knows how to load and sync it from a datafile
class FileLocationDatabase : public ILocationDatabase {
    static log4cxx::LoggerPtr m_logger;
    const std::string m_database_file;
    std::unordered_map<std::string, Location*> m_location_map;  
    std::vector<Location*> m_location_list;  
    
    FileLocationDatabase(const FileLocationDatabase& other); //copying of the Location database is dissalowed

protected:
    /// @brief This will delete all the locations in a given location list 
    /// @param locations The list to iterate over and free up the memory
    virtual void DeleteLocations(std::vector<Location*>& locations);

    /// @brief This will read the locations currently on disk
    /// @return The list of locations on disk 
    virtual std::vector<Location*> GetLocationsOnDisk();

    /// @brief This should add a location to the location database
    /// @param location The new location to add
    virtual void AddLocation(Location* location);
public:
    /// @brief This is the class constructor, taking a string file flor loading the database
    /// @param location_file 
    FileLocationDatabase(const std::string location_file);

    /// @brief This is the class destructor.
    virtual ~FileLocationDatabase();

    /// @brief This should load the location database, repeated calls should refresh the database. 
    /// @return True if the database was updated successfully, False if not
    bool Load();

    /// @brief This returns the current list of locations in the location database
    /// @return List of locations
    const std::vector<Location*> GetLocations() const;

    /// @brief This returns a particular location based on its name
    /// @param location_name The name of the location to get
    /// @return A location pointer
    Location* const GetLocation(std::string location_name) const;  
};

}

#endif