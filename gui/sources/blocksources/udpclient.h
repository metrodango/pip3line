#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QHostAddress>
#include <QDateTime>

class UDPClient {
    public:
        UDPClient();
        explicit UDPClient(QHostAddress clientAddress, quint16 clientPort);
        UDPClient(const UDPClient &other); // copy constructor
        QHostAddress getAdress() const;
        void setAdress(const QHostAddress &value);
        quint16 getPort() const;
        void setPort(const quint16 &value);
        bool operator==(const UDPClient& other) const;
        UDPClient& operator=(const UDPClient& other);
        QDateTime getCreationTimeStamp() const;

        int getSid() const;
        void setSid(int value);

    private:
        QHostAddress adress;
        quint16 port;
        QDateTime creationTimeStamp;
        int sid;
};

inline uint qHash(const UDPClient &key)
{
    return qHash(key.getAdress().toString()) ^ key.getPort();
}

#endif // UDPCLIENT_H
