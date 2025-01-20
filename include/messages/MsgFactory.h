#ifndef MSGFACTORY_H
#define MSGFACTORY_H

#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

namespace messages {

/// @brief This class knows how to create Messages
class MsgFactory {
public:
    typedef std::shared_ptr<MsgFactory> MsgFactoryPtr; /// Typedef for a MsgFactory pointer type

    /// @brief This method can create a message based upon its id
    /// @param id This is the id of the message to create
    /// @return The created message on success, or a nullptr on failure
    virtual MsgHeader::MsgPointer Create(const unsigned int id) const;
    
    /// @brief This method can create the message header
    /// @return A message header
    virtual MsgHeader::MsgPointer Header() const;

    /// @brief This method will return the lngth of the largest message
    /// @return The largets message size in bytes
    virtual size_t MaxLength() const;
};

}

#endif