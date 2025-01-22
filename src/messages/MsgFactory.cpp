#include <boost/asio.hpp>
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"

namespace messages {

log4cxx::LoggerPtr MsgFactory::m_logger(log4cxx::Logger::getLogger("MsgFactory"));

MsgHeader::MsgPointer MsgFactory::Create(const unsigned int id) const {
    switch(id)
    {
        case MSG_HEADER_ID:
            return MsgHeader::MsgPointer(new MsgHeader()); 
        case MSG_STATUS_REQUEST_ID:
            return MsgStatusRequest::MsgPointer(new MsgStatusRequest()); 
        case MSG_STATUS_RESPONSE_ID:
            return MsgStatusResponse::MsgPointer(new MsgStatusResponse()); 
        case MSG_LOCATIONS_REQUEST_ID:
            return MsgLocationsRequest::MsgPointer(new MsgLocationsRequest());
        case MSG_LOCATIONS_RESPONSE_ID:
            return MsgLocationsResponse::MsgPointer(new MsgLocationsResponse());
    }
    LOG4CXX_WARN(m_logger, "Unkown msg id=" << id);
    return MsgHeader::MsgPointer(nullptr);
}

MsgHeader::MsgPointer MsgFactory::Header() const {
    return Create(MSG_HEADER_ID);
}

size_t MsgFactory::MaxLength() const {
    return sizeof(MsgHeaderData) + sizeof(MsgLocationsResponseData);
}

}
