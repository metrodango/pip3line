#ifndef PACKETHANDLER_H
#define PACKETHANDLER_H

#include <QSharedPointer>
#include <pipelinecommon.h>
using namespace Pip3lineCommon;

class PacketHandler
{
    public:
        explicit PacketHandler();
        virtual ~PacketHandler();
        virtual void addPacket(QSharedPointer<Packet> packet) = 0;
};

#endif // PACKETHANDLER_H
