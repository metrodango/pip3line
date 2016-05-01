#include "udpclient.h"
#include "blockssource.h"

UDPClient::UDPClient() :
    sid(Block::INVALID_ID)
{
    port = 0;
    creationTimeStamp = QDateTime::currentDateTime();
}

UDPClient::UDPClient(QHostAddress clientAddress, quint16 clientPort) :
    sid(Block::INVALID_ID)
{
    adress = clientAddress;
    port = clientPort;
    creationTimeStamp = QDateTime::currentDateTime();
}

UDPClient::UDPClient(const UDPClient &other)
{
    adress = other.adress;
    port = other.port;
    creationTimeStamp = other.creationTimeStamp;
    sid = other.sid;
}

QHostAddress UDPClient::getAdress() const
{
    return adress;
}

void UDPClient::setAdress(const QHostAddress &value)
{
    adress = value;
}
quint16 UDPClient::getPort() const
{
    return port;
}

void UDPClient::setPort(const quint16 &value)
{
    port = value;
}

bool UDPClient::operator==(const UDPClient &other) const
{
    // only comparing the address and port number, the sid and timestamps are irrelevant in this case
    return other.adress == adress && other.port == port;
}

UDPClient& UDPClient::operator=(const UDPClient &other)
{
    adress = other.adress;
    port = other.port;
    sid = other.sid;
    creationTimeStamp = other.creationTimeStamp;
    return *this;
}
QDateTime UDPClient::getCreationTimeStamp() const
{
    return creationTimeStamp;
}
int UDPClient::getSid() const
{
    return sid;
}

void UDPClient::setSid(int value)
{
    sid = value;
}


