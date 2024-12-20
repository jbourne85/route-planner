#ifndef MSGFACTORY_H
#define MSGFACTORY_H

#include "messages/MsgHeader.h"
#include "messages/MsgStatus.h"

namespace messages {

MsgHeader::MsgPointer construct_message(const unsigned int id);

}

#endif