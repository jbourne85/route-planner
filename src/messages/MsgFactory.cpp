#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

std::shared_ptr<MsgHeader> construct_message(const unsigned int id)
{
    switch(id)
    {
        case MSG_HEADER_ID:
            return std::shared_ptr<MsgHeader>(new MsgHeader(MSG_HEADER_ID, sizeof(MsgHeader))); 
        case MSG_STATUS_REQUEST_ID:
            return std::shared_ptr<MsgHeader>(new MsgStatusRequest()); 
        case MSG_STATUS_RESPONSE_ID:
            return std::shared_ptr<MsgHeader>(new MsgStatusResponse()); 
    }
    return std::shared_ptr<MsgHeader>(nullptr);
}