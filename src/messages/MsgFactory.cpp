#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

MsgHeader::pointer construct_message(const unsigned int id)
{
    switch(id)
    {
        case MSG_HEADER_ID:
            return MsgHeader::pointer(new MsgHeader(MSG_HEADER_ID, sizeof(MsgHeader))); 
        case MSG_STATUS_REQUEST_ID:
            return MsgHeader::pointer(new MsgStatusRequest()); 
        case MSG_STATUS_RESPONSE_ID:
            return MsgHeader::pointer(new MsgStatusResponse()); 
    }
    return MsgHeader::pointer(nullptr);
}