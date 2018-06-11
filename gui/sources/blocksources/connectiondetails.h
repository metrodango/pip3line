#ifndef CONNECTIONDETAILS_H
#define CONNECTIONDETAILS_H

#include <QHostAddress>
#include <QDateTime>

class ConnectionDetails {
    public:
        ConnectionDetails();
        explicit ConnectionDetails(QHostAddress clientAddress, quint16 clientPort, bool tlsEnabled = false);
        ConnectionDetails(const ConnectionDetails &other); // copy constructor
        QHostAddress getAdress() const;
        void setAdress(const QHostAddress &value);
        quint16 getPort() const;
        void setPort(const quint16 &value);
        bool operator==(const ConnectionDetails& other) const;
        ConnectionDetails& operator=(const ConnectionDetails& other);
        QDateTime getCreationTimeStamp() const;
        int getSid() const;
        void setSid(int value);
        bool isTlsEnabled() const;
        void setTlsEnabled(bool value);
        QString getHostname() const;
        void setHostname(const QString &value);
    private:
        QHostAddress adress;
        quint16 port;
        bool tlsEnabled;
        QDateTime creationTimeStamp;
        int sid;
        QString hostname;
};

inline uint qHash(const ConnectionDetails &key)
{
    return qHash(key.getAdress()) ^ qHash(key.getPort());
}

#endif // CONNECTIONDETAILS_H
