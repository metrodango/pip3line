/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pipelinecommon.h"
#include <QDebug>

namespace Pip3lineCommon {
    const int Block::INVALID_ID = -1;

    Block::Block(QByteArray data, int sourceid):
        data(data),
        sourceid(sourceid),
        protocol(unknown_protocol),
        srcPort(0),
        dstPort(0),
        multiplexed(false)
    {

    }

    Block::~Block()
    {
        //qDebug() << "Destroying" << this;
    }

    QByteArray Block::getData() const
    {
        return data;
    }

    void Block::setData(const QByteArray &value)
    {
        data = value;
    }

    int Block::getSourceid() const
    {
        return sourceid;
    }

    void Block::setSourceid(int value)
    {
        sourceid = value;
    }

    DataProtocol Block::getProtocol() const
    {
        return protocol;
    }

    void Block::setProtocol(const DataProtocol &value)
    {
        protocol = value;
    }
    
    quint16 Block::getSrcPort() const
    {
        return srcPort;
    }
    
    void Block::setSrcPort(const quint16 &value)
    {
        srcPort = value;
    }

    quint16 Block::getDstPort() const
    {
        return dstPort;
    }

    void Block::setDstPort(const quint16 &value)
    {
        dstPort = value;
    }
    
    bool Block::isMultiplexed() const
    {
        return multiplexed;
    }
    
    void Block::setMultiplexed(bool value)
    {
        multiplexed = value;
    }
    
    const int Packet::USER_SOURCE = -1;
    
    Packet::Packet(QByteArray data, int sourceid) :
        originalData(data),
        data(data),
        sourceid(sourceid),
        timestamp(QDateTime::currentDateTime()),
        microsec(0),
        direction(NODIRECTION),
        injected(false),
        protocol(Pip3lineCommon::unknown_protocol),
        srcPort(0),
        dstPort(0)
    {

    }

    Packet::Packet(Block *block) :
        timestamp(QDateTime::currentDateTime()),
        microsec(0),
        direction(NODIRECTION),
        injected(false)
    {
        originalData = block->getData();
        data = originalData;
        sourceid = block->getSourceid();
        protocol = block->getProtocol();
        srcPort = block->getSrcPort();
        dstPort = block->getDstPort();
        multiplexed = block->isMultiplexed();
    }

    Packet::~Packet()
    {
       //  qDebug() << "Packet destroyed" << this;
        history.clear();
        sourceid = Block::INVALID_ID;
    }

    Packet *Packet::clone()
    {
        Packet * pac = new(std::nothrow) Packet();
        if (pac != nullptr) {
            pac->originalData = originalData;
            pac->data = data;
            pac->history = history;
            pac->sourceid = sourceid;
            pac->sourceString = sourceString;
            pac->timestamp = timestamp;
            pac->microsec = microsec;
            pac->direction = direction;
            pac->comment = comment;
            pac->foreground = foreground;
            pac->background = background;
            pac->injected = injected;
            pac->protocol = protocol;
            pac->srcPort = srcPort;
            pac->dstPort = dstPort;
            pac->multiplexed = multiplexed;
        }
        return pac;
    }

    QByteArray Packet::getOriginalData() const
    {
        return originalData;
    }

    void Packet::setOriginalData(const QByteArray &value, bool setDataAsWell)
    {
        if (originalData != value) {
            originalData = value;
            if (setDataAsWell)
                data = value;
            emit updated();
        }
    }

    QByteArray Packet::getData() const
    {
        return data;
    }

    void Packet::setData(const QByteArray &value, bool setOriginalAsWell)
    {
        if (data != value) {
            data = value;
            if (setOriginalAsWell)
                originalData = value;
            emit updated();
        }
    }
    QList<Pip3lineCommon::HistItem> Packet::getHistory() const
    {
        return history;
    }

    void Packet::setHistory(const QList<Pip3lineCommon::HistItem> &value)
    {
        history = value;
    }
    int Packet::getSourceid() const
    {
        return sourceid;
    }

    void Packet::setSourceid(int value)
    {
        sourceid = value;
    }
    QString Packet::getSourceString() const
    {
        return sourceString;
    }

    void Packet::setSourceString(const QString &value)
    {
        sourceString = value;
    }
    QDateTime Packet::getTimestamp() const
    {
        return timestamp;
    }

    void Packet::setTimestamp(const QDateTime &value)
    {
        timestamp = value;
    }
    Packet::Direction Packet::getDirection() const
    {
        return direction;
    }

    void Packet::setDirection(const Direction &value)
    {
        direction = value;
    }

    int Packet::size()
    {
        return data.size();
    }

    Block *Packet::toBlock()
    {
        Block * datab = new(std::nothrow) Block(data,sourceid);
        if (datab == nullptr) qFatal("Cannot allocate Block for Packet X{");
        datab->setProtocol(protocol);
        datab->setSrcPort(srcPort);
        datab->setDstPort(dstPort);
        datab->setMultiplexed(multiplexed);
        return datab;
    }

    bool Packet::hasBeenModified() const
    {
        return (originalData != data);
    }

    QHash<QString, QString> Packet::getAdditionalFields() const
    {
        return additionalFields;
    }

    void Packet::setAdditionalFields(const QHash<QString, QString> &value)
    {
        additionalFields = value;
    }

    QString Packet::fetchAdditionalField(const QString &fieldName) const
    {
        return additionalFields.value(fieldName,QString());
    }

    void Packet::setAdditionalField(const QString &FieldName, const QString &value)
    {
        additionalFields.insert(FieldName,value);
    }

    void Packet::removeAdditionalFields(const QString &FieldName)
    {
        if (additionalFields.contains(FieldName)) {
            additionalFields.remove(FieldName);
        } else {
            qCritical() << QObject::tr("[Packet::removeAdditionalFields] No field with name %1 T_T").arg(FieldName);
        }
    }
    QColor Packet::getForeground() const
    {
        return foreground;
    }

    void Packet::setForeground(const QColor &value)
    {
        foreground = value;
    }
    QColor Packet::getBackground() const
    {
        return background;
    }

    void Packet::setBackground(const QColor &value)
    {
        background = value;
    }
    quint32 Packet::getMicrosec() const
    {
        return microsec;
    }

    void Packet::setMicrosec(const quint32 &value)
    {
        microsec = value;
    }
    QString Packet::getComment() const
    {
        return comment;
    }

    void Packet::setComment(const QString &value)
    {
        comment = value;
    }
    bool Packet::isInjected() const
    {
        return injected;
    }

    void Packet::setInjected(bool value)
    {
        injected = value;
    }

    bool Packet::lessThanFunc(QSharedPointer<Packet> or1, QSharedPointer<Packet> or2)
    {
        if (or1->getTimestamp() == or2->getTimestamp())
            return or1->getMicrosec() < or2->getMicrosec();
        else
            return or1->getTimestamp() < or2->getTimestamp();
    }

    DataProtocol Packet::getProtocol() const
    {
        return protocol;
    }

    void Packet::setProtocol(const DataProtocol &value)
    {
        protocol = value;
    }

    quint16 Packet::getSrcPort() const
    {
        return srcPort;
    }

    void Packet::setSrcPort(const quint16 &value)
    {
        srcPort = value;
    }

    quint16 Packet::getDstPort() const
    {
        return dstPort;
    }

    void Packet::setDstPort(const quint16 &value)
    {
        dstPort = value;
    }
    
    bool Packet::isMultiplexed() const
    {
        return multiplexed;
    }
    
    void Packet::setMultiplexed(bool value)
    {
        multiplexed = value;
    }
}
