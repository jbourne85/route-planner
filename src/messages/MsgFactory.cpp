#include <boost/asio.hpp>
#include <iostream>
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

namespace messages {

MsgHeader::MsgPointer MsgFactory::Create(const unsigned int id) const {
    switch(id)
    {
        case MSG_HEADER_ID:
            return MsgHeader::MsgPointer(new MsgHeader(MSG_HEADER_ID, MSG_HEADER_SIZE)); 
        case MSG_STATUS_REQUEST_ID:
            return MsgHeader::MsgPointer(new MsgStatusRequest()); 
        case MSG_STATUS_RESPONSE_ID:
            return MsgHeader::MsgPointer(new MsgStatusResponse()); 
    }
    return MsgHeader::MsgPointer(nullptr);
}

MsgHeader::MsgPointer MsgFactory::Header() const {
    return Create(MSG_HEADER_ID);
}

}