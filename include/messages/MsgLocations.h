#ifndef MSGLOCATIONS_H_
#define MSGLOCATIONS_H_

#include "messages/MsgHeader.h"

namespace messages {

const unsigned int MSG_LOCATIONS_REQUEST_ID = 103;
const unsigned int MSG_LOCATIONS_RESPONSE_ID = 104;
const size_t MSG_LOCATIONS_MAX_CHARS = 100;
const char MSG_LOCATIONS_SEP_CHAR = ';';

/// @brief This is the low level Locations Request Message, it used to request the complete list
///        of locations avaliable in the db
struct MsgLocationsRequest : MsgHeader {
    typedef std::shared_ptr<MsgLocationsRequest> MsgPointer; //typedef for a message pointer

    MsgLocationsRequest() : MsgHeader(MSG_LOCATIONS_REQUEST_ID, sizeof(MsgLocationsRequest)){}
};


/// @brief This is the response message for the list of locations in the db
struct MsgLocationsResponse : MsgHeader {
    size_t char_count;                          /// This is the number of chars contained in this message
    char locations[MSG_LOCATIONS_MAX_CHARS];    /// This is a char array containing the list of locations sperated by a \n char
    bool is_paginated;                          /// This signals if there are more locations to be fetched

    typedef std::shared_ptr<MsgLocationsResponse> MsgPointer; //typedef for a message pointer
    
    /// @brief This is the class contructor
    MsgLocationsResponse() : 
    MsgHeader(MSG_LOCATIONS_RESPONSE_ID, 
    sizeof(MsgLocationsResponse)), 
    char_count(0), 
    is_paginated(false)
    {
        memset(locations, ' ', MSG_LOCATIONS_MAX_CHARS);
    }

    /// @brief This will add a string representation of a location to the message
    /// @param location_name This is the string location name to add
    /// @return True if the location was added to the message, false if not with is_paginated set to true
    bool AddLocation(std::string& location_name) {
        size_t new_char_count = char_count + location_name.size();
        if (new_char_count < MSG_LOCATIONS_MAX_CHARS) {
            std::memcpy(&locations[char_count], location_name.c_str(), location_name.size());
            locations[new_char_count] = MSG_LOCATIONS_SEP_CHAR;
            char_count = ++new_char_count;
            return true;
        }
        else {
            is_paginated = true;
            return false;
        }
    }

    /// @brief This will get the locations the message contains as a list of strings
    /// @return The list of locations
    const std::vector<std::string> GetLocations() {
        std::string temp_str(locations, char_count);
        std::vector<std::string> location_list;

        size_t pos = 0;
        std::string token;
        while((pos = temp_str.find(MSG_LOCATIONS_SEP_CHAR)) != std::string::npos) {
            token = temp_str.substr(0, pos);
            location_list.push_back(token);
            temp_str.erase(0, pos + sizeof(MSG_LOCATIONS_SEP_CHAR));
        }
        location_list.push_back(temp_str);

        return location_list;
    }
};

}

#endif