#ifndef MSGFACTORY_H
#define MSGFACTORY_H

#include "messages/MsgHeader.h"

MsgHeader::pointer construct_message(const unsigned int id);

#endif