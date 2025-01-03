#ifndef MSGSTATUS_H_
#define MSGSTATUS_H_

#include "MsgHeader.h"

namespace messages {

const unsigned int MSG_STATUS_REQUEST_ID = 101;
const unsigned int MSG_STATUS_RESPONSE_ID = 102;

/// @brief This is the low level Status Request Message, it used to request that the server is up and 
///        listning for requests
struct MsgStatusRequest : MsgHeader {
    MsgStatusRequest() : MsgHeader(MSG_STATUS_REQUEST_ID, sizeof(MsgStatusRequest)){}
};


/// @brief This is the low level Status Response Message, it used to respond when the server is up and running
struct MsgStatusResponse : MsgHeader {
    MsgStatusResponse() : MsgHeader(MSG_STATUS_RESPONSE_ID, sizeof(MsgStatusResponse)){}
};

}

#endif