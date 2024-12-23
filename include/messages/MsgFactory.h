#ifndef MSGFACTORY_H
#define MSGFACTORY_H

#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

namespace messages {

/// @brief This class knows how to create Messages
class MsgFactory {
public:
    /// @brief This method can create a message based upon its id
    /// @param id This is the id of the message to create
    /// @return The created message on success, or a nullptr on failure
    MsgHeader::MsgPointer create(const unsigned int id);
};
}

#endif