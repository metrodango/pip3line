#include "connectiondetails.h"
#include "blockssource.h"

ConnectionDetails::ConnectionDetails() :
    port(0),
    tlsEnabled(false),
    creationTimeStamp(QDateTime::currentDateTime()),
    sid(Block::INVALID_ID)
{

}

ConnectionDetails::ConnectionDetails(QHostAddress clientAddress, quint16 clientPort, bool tlsEnabled) :
    adress(clientAddress),
    port(clientPort),
    tlsEnabled(tlsEnabled),
    creationTimeStamp(QDateTime::currentDateTime()),
    sid(Block::INVALID_ID)
{

}

ConnectionDetails::ConnectionDetails(const ConnectionDetails &other)
{
    adress = other.adress;
    port = other.port;
    creationTimeStamp = other.creationTimeStamp;
    tlsEnabled = other.tlsEnabled;
    sid = other.sid;
    hostname = other.hostname;
}

QHostAddress ConnectionDetails::getAdress() const
{
    return adress;
}

void ConnectionDetails::setAdress(const QHostAddress &value)
{
    adress = value;
}
quint16 ConnectionDetails::getPort() const
{
    return port;
}

void ConnectionDetails::setPort(const quint16 &value)
{
    port = value;
}

bool ConnectionDetails::operator==(const ConnectionDetails &other) const
{
    // only comparing the address and port number, the sid and timestamps are irrelevant in this case
    return other.adress == adress && other.port == port;
}

ConnectionDetails& ConnectionDetails::operator=(const ConnectionDetails &other)
{
    adress = other.adress;
    port = other.port;
    sid = other.sid;
    tlsEnabled = other.tlsEnabled;
    creationTimeStamp = other.creationTimeStamp;
    hostname = other.hostname;
    return *this;
}
QDateTime ConnectionDetails::getCreationTimeStamp() const
{
    return creationTimeStamp;
}
int ConnectionDetails::getSid() const
{
    return sid;
}

void ConnectionDetails::setSid(int value)
{
    sid = value;
}

bool ConnectionDetails::isTlsEnabled() const
{
    return tlsEnabled;
}

void ConnectionDetails::setTlsEnabled(bool value)
{
    tlsEnabled = value;
}

QString ConnectionDetails::getHostname() const
{
    return hostname;
}

void ConnectionDetails::setHostname(const QString &value)
{
    hostname = value;
}


