#ifndef MSGHEADER_H_
#define MSGHEADER_H_

#include <boost/asio.hpp>
#include <cstdlib>
#include <vector>

namespace messages {
/**
 * Generic Message header class
 */
struct MsgHeader {
    const unsigned int id;        //The ID of the message
    const unsigned int length;    //The length of the message (in bytes)
    const time_t timestamp;       //This is the epoch timestamp of the message     

    typedef std::shared_ptr<MsgHeader> MsgPointer; //typedef for a message header pointer
    typedef std::function<MsgHeader::MsgPointer(MsgHeader::MsgPointer)> MsgHandler; //typedef for a mesage handler function

    MsgHeader(const unsigned int id, const unsigned int length) : id(id), length(length), timestamp(std::time(0)) {}
    
    /** 
    * This method can be used to serialize a Msg
    * object into a char buffer
    * @param buffer the char buffer to serialize into
    */
    void serialize(std::vector<char>& buffer) const 
    {
        buffer.clear();        
        buffer.resize(length);
        std::memcpy(buffer.data(), this, length);
    }

    /** 
    * This method can be used to deserialize a Msg
    * object from a char buffer
    * @param buffer the char buffer to serialize from
    */
    void deserialize(std::vector<char>& buffer)
    {
        std::memcpy(this, buffer.data(), length);
    }
};

const unsigned int MSG_HEADER_ID = 0; 
const size_t MSG_HEADER_SIZE = sizeof(MsgHeader);

}
#endif