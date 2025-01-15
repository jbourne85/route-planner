#ifndef MSGLOCATIONS_H_
#define MSGLOCATIONS_H_

#include "messages/MsgHeader.h"

namespace messages {

const unsigned int MSG_LOCATIONS_REQUEST_ID = 103;
const unsigned int MSG_LOCATIONS_RESPONSE_ID = 104;
const size_t MSG_LOCATIONS_MAX_CHARS = 100;

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
    {}

    /// @brief This will add a string representation of a location to the message
    /// @param location_name This is the string location name to add
    /// @return True if the location was added to the message, false if not with is_paginated set to true
    bool AddLocation(std::string& location_name) {
        size_t new_char_count = char_count + location_name.size() + 1;
        if (new_char_count < MSG_LOCATIONS_MAX_CHARS) {
            std::memcpy(&locations[char_count], location_name.c_str(), location_name.size());
            locations[location_name.size() + 1] = '\n';
            return true;
        }
        else {
            is_paginated = true;
            return false;
        }
    }
};

}

#endif