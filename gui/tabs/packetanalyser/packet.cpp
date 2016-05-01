/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "packet.h"
#include <QDateTime>
#include <QDebug>
#include "sources/blocksources/blockssource.h"

const int Packet::USER_SOURCE = -1;

Packet::Packet(QByteArray data, int sourceid) :
    originalData(data),
    data(data),
    sourceid(sourceid),
    timestamp(QDateTime::currentDateTime()),
    microsec(0),
    direction(NODIRECTION),
    injected(false)
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
}

Packet::~Packet()
{
   //  qDebug() << "Packet destroyed" << this;
    history.clear();
    sourceid = Block::INVALID_ID;
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
QList<ByteSourceAbstract::HistItem> Packet::getHistory() const
{
    return history;
}

void Packet::setHistory(const QList<ByteSourceAbstract::HistItem> &value)
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













