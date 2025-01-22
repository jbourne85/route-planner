#include <boost/bind/bind.hpp>
#include "comms/TcpMsgMatch.h"

namespace boost {
    namespace asio {
        using namespace std::placeholders;

        template <> struct is_match_condition<
            decltype(
                boost::bind(
                        &comms::TcpMsgMatch::ProcessBuffer,
                        comms::TcpMsgMatch::MsgMatchPointer(), 
                        _1, 
                        _2
                )
            )>
            : public boost::true_type {};
    }
}

namespace comms {

using messages::MsgHeader;

log4cxx::LoggerPtr TcpMsgMatch::m_logger(log4cxx::Logger::getLogger("TcpMsgMatch"));

TcpMsgMatch::TcpMsgMatch(messages::MsgFactory::MsgFactoryPtr msg_factory) : 
m_total_bytes_read(0),
m_bytes_received(0),
m_msg_factory(msg_factory),
m_msg_header(m_msg_factory->Create(messages::MSG_HEADER_ID)),
m_msg(nullptr),
m_header_length(m_msg_header->Length())
{}

std::pair<TcpMsgMatch::MsgBufferIterator, bool> TcpMsgMatch::ProcessBuffer(TcpMsgMatch::MsgBufferIterator begin, TcpMsgMatch::MsgBufferIterator end) {

    m_total_bytes_read += end - begin;

    m_buffer.insert(m_buffer.end(), begin, end);

    if (m_total_bytes_read >= m_header_length) {
        m_msg_header->Deserialize(m_buffer);
        LOG4CXX_DEBUG(m_logger, "Received msg header. total_bytes_read=" << m_total_bytes_read << " header.id=" << m_msg_header->Id() << " header.length=" << m_header_length);
        m_msg = m_msg_factory->Create(m_msg_header->Id());

        if (m_msg && m_total_bytes_read >= m_msg->Length()) {
            m_msg->Deserialize(m_buffer);
            LOG4CXX_DEBUG(m_logger, "Received msg body. total_bytes_read=" << m_total_bytes_read << " header.id=" << m_msg_header->Id() << " header.length=" << m_msg->Length());
            return std::make_pair(end, true); 
        }
        else {
            LOG4CXX_DEBUG(m_logger, "Waiting to receive msg body. total_bytes_read=" << m_total_bytes_read << " header.length=" << m_header_length);
        }
    }
    else {
        LOG4CXX_DEBUG(m_logger, "Waiting to receive msg header. total_bytes_read=" << m_total_bytes_read << " header.length=" << m_header_length);
    }

    return std::make_pair(end, false); 
}

MsgHeader::MsgPointer TcpMsgMatch::GetMsg() {
    return m_msg;
}

}