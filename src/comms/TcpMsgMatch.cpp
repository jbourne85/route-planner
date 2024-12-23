#include <boost/bind/bind.hpp>
#include "comms/TcpMsgMatch.h"

namespace boost {
    namespace asio {
        using namespace std::placeholders;

        template <> struct is_match_condition<
            decltype(
                boost::bind(
                        &comms::TcpMsgMatch::match_found,
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

TcpMsgMatch::TcpMsgMatch() : 
m_total_bytes_read(0),
m_bytes_received(0),
m_msg_factory(),
m_msg_header(m_msg_factory.create(messages::MSG_HEADER_ID)),
m_msg(nullptr)
{}

std::pair<TcpMsgMatch::MsgBufferIterator, bool> TcpMsgMatch::match_found(TcpMsgMatch::MsgBufferIterator begin, TcpMsgMatch::MsgBufferIterator end) {

    m_total_bytes_read += end - begin;

    m_buffer.insert(m_buffer.end(), begin, end);

    if (m_total_bytes_read >= m_msg_header->length) {
        m_msg_header->deserialize(m_buffer);
        m_msg = m_msg_factory.create(m_msg_header->id);
    
        if (m_msg && m_total_bytes_read >= m_msg->length) {
            m_msg->deserialize(m_buffer);
            return std::make_pair(end, true); 
        }
    }

    return std::make_pair(end, false); 
}

MsgHeader::MsgPointer TcpMsgMatch::get_match() {
    return m_msg;
}

}