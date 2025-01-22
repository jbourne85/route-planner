#ifndef TCPMSGMATCH_H
#define TCPMSGMATCH_H

#include <boost/asio.hpp>
#include <log4cxx/logger.h>
#include "messages/MsgFactory.h"
#include "messages/MsgHeader.h"

namespace comms {

/// @brief This class knows how to look at a boost message buffer and look for/create any messages that it may contain
class TcpMsgMatch {
    static log4cxx::LoggerPtr m_logger;
    size_t m_total_bytes_read;
    size_t m_bytes_received;
    std::vector<char> m_buffer;
    messages::MsgFactory::MsgFactoryPtr m_msg_factory;
    messages::MsgHeader::MsgPointer m_msg_header;
    messages::MsgHeader::MsgPointer m_msg;
    const size_t m_header_length;
        
public:
    typedef boost::asio::buffers_iterator<boost::asio::streambuf::const_buffers_type> MsgBufferIterator;
    typedef std::shared_ptr<TcpMsgMatch> MsgMatchPointer;
    
    TcpMsgMatch(messages::MsgFactory::MsgFactoryPtr msg_factory);
    
    /// @brief This will check to see if a matching message has been found within two boost::asio::buffers_iterator's. It can be called
    ///        as many times as required until a complete message is found. It is intended to be used with boost::asio::async_read_until
    /// @param begin This is the current start of the incoming data to check
    /// @param end This is the current end of the incoming data to check
    /// @return returns a std::pair of iterator and bool (as defined by the boost::asio::async_read_until method)
    std::pair<TcpMsgMatch::MsgBufferIterator, bool> ProcessBuffer(TcpMsgMatch::MsgBufferIterator begin, TcpMsgMatch::MsgBufferIterator end);


    /// @brief Get's the current found message.
    /// @return The found message on success, or a nullptr on failure
    messages::MsgHeader::MsgPointer GetMsg();
};

}

#endif