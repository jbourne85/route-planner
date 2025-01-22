#ifndef MSGLOCATIONS_H_
#define MSGLOCATIONS_H_

#include "messages/MsgHeader.h"

namespace messages {

const unsigned int MSG_LOCATIONS_REQUEST_ID = 103;
const unsigned int MSG_LOCATIONS_RESPONSE_ID = 104;
const size_t MSG_LOCATIONS_MAX_CHARS = 100;
const char MSG_LOCATIONS_SEP_CHAR = ';';

/// @brief This is the Locations request to get a list of locations from the server
struct MsgLocationsRequestData {
    size_t start_location;  /// This is the index of the locations on the server to fetch, by default wil be 0

    MsgLocationsRequestData() :
    start_location(0) {
    }
};

/// @brief This is the Locations request Msg
class MsgLocationsRequest : public MsgHeader {
    MsgLocationsRequestData m_msg;
public: 
    typedef std::shared_ptr<MsgLocationsRequest> MsgPointer;        ///typedef for a derived message pointer

    MsgLocationsRequest() : 
    MsgHeader(MSG_LOCATIONS_REQUEST_ID, sizeof(MsgLocationsRequestData), 
    (char* const)&m_msg), 
    m_msg() {
    }

    /// @brief This will set the start location index in the data msg
    /// @param index This is start location index
    void SetStartLocation(size_t index) {
        m_msg.start_location = index;
    }

    /// @brief This will get a pointer to the underlying data structure
    /// @return Data structure pointer
    const MsgLocationsRequestData* const GetData() {
        return &m_msg;
    }
};

/// @brief This is the Locations response data for the list of locations in the db
struct MsgLocationsResponseData {
    size_t char_count;                          /// This is the number of chars contained in this message
    char locations[MSG_LOCATIONS_MAX_CHARS];    /// This is a char array containing the list of locations sperated by a \n char
    bool is_paginated;                          /// This signals if there are more locations to be fetched

    /// @brief This is the class contructor
    MsgLocationsResponseData() : 
    char_count(0), 
    is_paginated(false)
    {
        memset(locations, ' ', MSG_LOCATIONS_MAX_CHARS);
    }
};

/// @brief This is the Locations response Msg
class MsgLocationsResponse : public MsgHeader {
    MsgLocationsResponseData m_msg;
public:
    typedef std::shared_ptr<MsgLocationsResponse> MsgPointer;   ///typedef for a derived message pointer
    
    MsgLocationsResponse() : 
    MsgHeader(MSG_LOCATIONS_RESPONSE_ID, sizeof(MsgLocationsResponseData), 
    (char* const)&m_msg), 
    m_msg() {
    }

    /// @brief This will add a string representation of a location to the message
    /// @param location_name This is the string location name to add
    /// @return True if the location was added to the message, false if not with is_paginated set to true
    bool AddLocation(std::string& location_name) { 
        size_t new_char_count = m_msg.char_count + location_name.size();
        if (new_char_count < MSG_LOCATIONS_MAX_CHARS) {
            std::memcpy(&(m_msg.locations[m_msg.char_count]), location_name.c_str(), location_name.size());
            m_msg.locations[new_char_count] = MSG_LOCATIONS_SEP_CHAR;
            m_msg.char_count = ++new_char_count;
            return true;
        }
        else {
            m_msg.is_paginated = true;
            return false;
        }
    }

    /// @brief This will get the locations the message contains as a list of strings
    /// @return The list of locations
    const std::vector<std::string> GetLocations() {
        std::string temp_str(m_msg.locations, m_msg.char_count);
        std::vector<std::string> location_list;

        size_t pos = 0;
        std::string token;
        while((pos = temp_str.find(MSG_LOCATIONS_SEP_CHAR)) != std::string::npos) {
            token = temp_str.substr(0, pos);
            location_list.push_back(token);
            temp_str.erase(0, pos + sizeof(MSG_LOCATIONS_SEP_CHAR));
        }
        return location_list;
    }

    /// @brief This will get a pointer to the underlying data structure
    /// @return Data structure pointer
    const MsgLocationsResponseData* const GetData() {
        return &m_msg;
    }
};

}

#endif