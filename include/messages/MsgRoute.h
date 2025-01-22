#ifndef MSGROUTE_H_
#define MSGROUTE_H_

#include "messages/MsgHeader.h"

namespace messages {

const unsigned int MSG_ROUTE_REQUEST_ID = 105;
const unsigned int MSG_ROUTE_RESPONSE_ID = 106;

/// @brief This is the Route request data, to request a route calculation between a start and end location
struct MsgRouteRequestData {
    size_t start_location;   /// This is the index of the start location within the locations list
    size_t end_location;     /// This is the index of the end location within the location list

    MsgRouteRequestData() :
    start_location(0),
    end_location(1) {
    }
};

/// @brief This is the Route request Msg
class MsgRouteRequest : public MsgHeader {
    MsgRouteRequestData m_msg;

public: 
    typedef std::shared_ptr<MsgRouteRequest> MsgPointer;        ///typedef for a derived message pointer

    MsgRouteRequest() : 
    MsgHeader(MSG_ROUTE_REQUEST_ID, sizeof(MsgRouteRequestData), 
    (char* const)&m_msg), 
    m_msg() {
    }

    /// @brief This will set the start location index in the data msg
    /// @param index This is start location index
    void SetStartLocation(size_t index) {
        m_msg.start_location = index;
    }

    /// @brief This will set the end location index in the data msg
    /// @param index This is end location index
    void SetEndLocation(size_t index) {
        m_msg.end_location = index;
    }

    /// @brief This will get a pointer to the underlying data structure
    /// @return Data structure pointer
    const MsgRouteRequestData* const GetData() {
        return &m_msg;
    }
};

/// @brief This is the Route respponse data
struct MsgRouteResponseData {
    size_t cost;   /// This is the index of the start location within the locations list

    MsgRouteResponseData() :
    cost(0) {
    }
};

/// @brief This is the Route response Msg
class MsgRouteResponse : public MsgHeader {
    MsgRouteResponseData m_msg;

public: 
    typedef std::shared_ptr<MsgRouteResponse> MsgPointer;        ///typedef for a derived message pointer

    MsgRouteResponse() : 
    MsgHeader(MSG_ROUTE_RESPONSE_ID, sizeof(MsgRouteResponseData), 
    (char* const)&m_msg), 
    m_msg() {
    }


    /// @brief This will set the route calculation cost in the data msg
    /// @param index This is route calculation cost
    void SetCost(size_t cost) {
        m_msg.cost = cost;
    }

    /// @brief This will get a pointer to the underlying data structure
    /// @return Data structure pointer
    const MsgRouteResponseData* const GetData() {
        return &m_msg;
    }
};

}

#endif