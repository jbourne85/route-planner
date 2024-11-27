#ifndef MSGHEADER_H_
#define MSGHEADER_H_

#include <cstdlib>
#include <vector>

/**
 * Generic Message header class
 */
struct MsgHeader
{
    const unsigned int id;        //The ID of the message
    const unsigned int length;    //The length of the message (in bytes)
    const time_t timestamp;       //This is the epoch timestamp of the message     

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

const size_t MSG_HEADER_SIZE = sizeof(MsgHeader);

#endif