#ifndef MSGHEADER_H_
#define MSGHEADER_H_

#include <algorithm> 
#include <cstdlib>
#include <boost/asio.hpp>
#include <vector>

namespace messages {

/// @brief Generic Message header class
struct MsgHeader {
    const unsigned int id;        ///The ID of the message
    const size_t length;          ///The length of the message (in bytes)
    const time_t timestamp;       ///This is the epoch timestamp of the message     

    typedef std::shared_ptr<MsgHeader> MsgPointer; //typedef for a message header pointer
    typedef std::function<MsgHeader::MsgPointer(MsgHeader::MsgPointer)> MsgHandler; //typedef for a mesage handler function

    MsgHeader(const unsigned int id, const size_t length) : id(id), length(length), timestamp(std::time(0)) {}
    

    /// @brief This method can be used to serialize a Msg
    /// @param buffer the char buffer to serialize into
    void Serialize(std::vector<char>& buffer) const 
    {
        buffer.clear();        
        buffer.resize(length);
        std::memcpy(buffer.data(), this, length);
    }

    /// @brief This method can be used to deserialize a Msg object from a char buffer
    /// @param buffer the char buffer to serialize from
    /// @return The number of bytes that have be successfully deserialised
    size_t Deserialize(std::vector<char>& buffer)
    {
        std::memcpy(this, buffer.data(), std::min(length, buffer.size()));
        return buffer.size();
    }
};

const unsigned int MSG_HEADER_ID = 100; 
const size_t MSG_HEADER_SIZE = sizeof(MsgHeader);

}
#endif