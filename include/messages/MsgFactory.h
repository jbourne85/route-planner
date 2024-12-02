#ifndef MSGFACTORY_H
#define MSGFACTORY_H

#include "messages/MsgHeader.h"

std::shared_ptr<MsgHeader> construct_message(const unsigned int id);

#endif