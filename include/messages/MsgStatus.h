#ifndef MSGSTATUS_H_
#define MSGSTATUS_H_

#include "messages/MsgHeader.h"

namespace messages {

const unsigned int MSG_STATUS_REQUEST_ID = 101;
const unsigned int MSG_STATUS_RESPONSE_ID = 102;

/// @brief This is the Status request Msg
class MsgStatusRequest : public MsgHeader {
public:
    typedef std::shared_ptr<MsgStatusRequest> MsgPointer; ///typedef for a derived message pointer

    MsgStatusRequest() : MsgHeader(MSG_STATUS_REQUEST_ID) {}
};

/// @brief This is the Status response Msg
class MsgStatusResponse : public MsgHeader {
public:
    typedef std::shared_ptr<MsgStatusResponse> MsgPointer; ///typedef for a derived message pointer

    MsgStatusResponse() : MsgHeader(MSG_STATUS_RESPONSE_ID) {}
};

}

#endif