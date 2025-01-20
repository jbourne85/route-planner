#ifndef MSGHEADER_H_
#define MSGHEADER_H_

#include <algorithm> 
#include <chrono>
#include <cstdlib>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <vector>

namespace messages {

/// @brief Generic Message header data class
struct MsgHeaderData {
    const unsigned int id;        ///The ID of the message
    const size_t length;          ///The length of the message (in bytes)
    const time_t timestamp;       ///This is the epoch timestamp of the message     

    /// @brief This is the constructor for the Message Header data class
    /// @param id This is the id of the message
    /// @param length This is the length (in bytes) of the message
    MsgHeaderData(const unsigned int id, const size_t length) : 
    id(id), 
    length(length), 
    timestamp(std::time(0)) {
    }
};

const unsigned int MSG_HEADER_ID = 100; 
const size_t MSG_HEADER_SIZE = sizeof(MsgHeaderData);

class MsgHeader {
    const size_t m_header_length;   /// This is a member variable to hold the length (in bytes) of the MsgHeaderData
    const MsgHeaderData m_header;   /// This is an instance of the MsgHeaderData to indicate the message id/length
    const size_t m_body_length;     /// This is a member variable to hold the length (in bytes) of the Msg Body
    char* const m_body;             /// This is a member variable to point to the Msg Body data
protected:
    

public:
    typedef std::shared_ptr<MsgHeader> MsgPointer;                                  ///typedef for a message header pointer
    typedef std::function<MsgHeader::MsgPointer(MsgHeader::MsgPointer)> MsgHandler; ///typedef for a mesage handler function

    /// @brief This is the default constructor, it creates a Msg that has no body
    MsgHeader() : 
    m_header_length(sizeof(MsgHeaderData)),
    m_header(MSG_HEADER_ID, MSG_HEADER_SIZE), 
    m_body_length(0),
    m_body(nullptr) {
    }
    
    virtual ~MsgHeader() {}

    /// @brief This gets the Id of the msg
    const unsigned int Id() const {
        return m_header.id;
    }    

    /// @brief This gets the Length of the msg (including the header, and body if its set)
    const size_t Length() const {
        return m_header.length;
    } 

    /// @brief This gets the epoch Timestamp of the msg
    const time_t Timestamp() const {
        return m_header.timestamp;
    } 

    /// @brief This gets the epoch Timestamp as a Date String
    std::string DateString() const {
        return std::ctime(&m_header.timestamp);
    }

    /// @brief This method will serialize a this msg
    /// @param buffer the char buffer to serialize into, it will be resized to accomodate the message
    virtual void Serialize(std::vector<char>& buffer) {
        buffer.clear();        
        buffer.resize(m_header.length);        
        
        //serialize the header
        std::memcpy(buffer.data(), (char*)&m_header, m_header_length);

        //serialize the body (if we have one)
        if(m_body) {
            std::memcpy(buffer.data() + m_header_length, m_body, m_body_length);    
        }
    }

    /// @brief This method will deserialize a msg object from a char buffer, it will only deserialize the header and then body if there is enough data (respectivley)
    /// @param buffer the char buffer to serialize from
    /// @return The number of bytes that have be successfully deserialised (either 0 on nothing, size of the header, or the size of the header +_body)
    virtual size_t Deserialize(std::vector<char>& buffer) {
        size_t deserialized_bytes = 0;
        if (buffer.size() >= m_header_length) {            
            //deserialize the header
            std::memcpy((char*)&m_header, buffer.data(), m_header_length);
            deserialized_bytes = m_header_length;

            //deserialize the body (if we have one and enough data)
            if(m_header.length && buffer.size() >= m_header.length && m_body) {
                std::memcpy(m_body, buffer.data() + m_header_length, m_body_length);    
                deserialized_bytes = m_header.length;
            }
        }
        return deserialized_bytes;        
    }

    /// @brief Converts the current header instance into a derived class
    /// @tparam MsgDerivedType This is the derived class type to attempt to convert to
    /// @param msg This is the base msg pointer type
    /// @return A valid derived class pointer on success, nullptr on failure
    template<typename MsgDerivedType>
    static typename MsgDerivedType::MsgPointer GetDerivedType(MsgPointer msg) {
        return std::dynamic_pointer_cast<MsgDerivedType>(msg);
    }

protected:
    /// @brief This constructor is to be used by derived classes that represent msg's with data
    /// @param id The Id to use for the message
    /// @param body_length The Length of the derived classes data (in bytes)
    /// @param body_data Pointer to the derived classes data
    MsgHeader(const unsigned int id, const size_t body_length, char* const body_data) : 
    m_header_length(sizeof(MsgHeaderData)), 
    m_header(id, m_header_length + body_length), 
    m_body_length(body_length),
    m_body(body_data) {
    }

    /// @brief This constructor is to be used by derived classes that don't have any extra information past the header
    /// @param id The Id to use for the message
    MsgHeader(const unsigned int id) :
    m_header_length(sizeof(MsgHeaderData)), 
    m_header(id, m_header_length), 
    m_body_length(0),
    m_body(nullptr) {
    }
};

}
#endif