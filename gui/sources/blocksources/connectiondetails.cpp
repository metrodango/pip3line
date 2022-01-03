#include "connectiondetails.h"
#include "blockssource.h"

ConnectionDetails::ConnectionDetails() :
    port(0),
    tlsEnabled(false),
    creationTimeStamp(QDateTime::currentDateTime()),
    lastPacketTimeStamp(creationTimeStamp),
    sid(Block::INVALID_ID),
    dtlscontext(QSharedPointer<QDtls>()),
    timeoutRetry(0)
{

}

ConnectionDetails::ConnectionDetails(QHostAddress clientAddress, quint16 clientPort, bool tlsEnabled) :
    address(clientAddress),
    port(clientPort),
    tlsEnabled(tlsEnabled),
    creationTimeStamp(QDateTime::currentDateTime()),
    lastPacketTimeStamp(creationTimeStamp),
    sid(Block::INVALID_ID),
    dtlscontext(QSharedPointer<QDtls>()),
    timeoutRetry(0)
{

}

ConnectionDetails::ConnectionDetails(const ConnectionDetails &other)
{
    address = other.address;
    port = other.port;
    creationTimeStamp = other.creationTimeStamp;
    lastPacketTimeStamp = other.lastPacketTimeStamp;
    tlsEnabled = other.tlsEnabled;
    sid = other.sid;
    hostname = other.hostname;
    dtlscontext = other.dtlscontext;
}

ConnectionDetails::~ConnectionDetails()
{

}

QHostAddress ConnectionDetails::getAddress() const
{
    return address;
}

void ConnectionDetails::setAddress(const QHostAddress &value)
{
    address = value;
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
    return other.address == address && other.port == port;
}

ConnectionDetails& ConnectionDetails::operator=(const ConnectionDetails &other)
{
    address = other.address;
    port = other.port;
    sid = other.sid;
    tlsEnabled = other.tlsEnabled;
    creationTimeStamp = other.creationTimeStamp;
    lastPacketTimeStamp = other.lastPacketTimeStamp;
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

QSharedPointer<QDtls> ConnectionDetails::getDtlscontext() const
{
    return dtlscontext;
}

void ConnectionDetails::setDtlscontext(const QSharedPointer<QDtls> &value)
{
    dtlscontext = value;
}

void ConnectionDetails::bumpLastTimestamp()
{
    lastPacketTimeStamp = QDateTime::currentDateTime();
}

QDateTime ConnectionDetails::getLastPacketTimeStamp() const
{
    return lastPacketTimeStamp;
}

int ConnectionDetails::getTimeoutRetry() const
{
    return timeoutRetry;
}

void ConnectionDetails::setTimeoutRetry(int value)
{
    timeoutRetry = value;
}

void ConnectionDetails::incrTimeoutRetries()
{
    timeoutRetry++;
}

int ConnectionDetailsList::connectionIndex(const QHostAddress &clientAddress, const quint16 &clientPort)
{
    int ret = -1;
    for (int i = 0; i < size(); i++) {
        QSharedPointer<ConnectionDetails> c = at(i);
        if (c->getAddress() == clientAddress && c->getPort() == clientPort) {
            ret = i;
            break;
        }
    }
    return ret;
}
