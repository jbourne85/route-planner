#ifndef ILOCATIONDATABASE_H
#define ILOCATIONDATABASE_H

#include "route/Location.h"

namespace route {

/// @brief This should represent a source of locations, it should be responsible for the 
///        lifetime of these locations
class ILocationDatabase {
    ILocationDatabase(const ILocationDatabase& other) {} //copying of this is dissallowed

public:
    ILocationDatabase() {}
    virtual ~ILocationDatabase() {}

    /// @brief This should load the location database, repeated calls will reload the database. If an error occurrs on loading
    ///        the previous list of locations will be kept
    /// @return True if the locations were updated successfully, False if not
    virtual bool Load() = 0;

    /// @brief This should load all the locations from the source the concrete version of this
    ///        class represents
    /// @return A list of pointers to the loaded locations
    virtual const std::vector<Location*> GetLocations() const = 0;

    /// @brief This should returns a particular location based on its name
    /// @param location_name The name of the location to get
    /// @return A location pointer
    virtual Location* const GetLocation(std::string location_name) const = 0;  

};
}

#endif