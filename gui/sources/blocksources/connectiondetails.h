#ifndef CONNECTIONDETAILS_H
#define CONNECTIONDETAILS_H

#include <QHostAddress>
#include <QDateTime>
#include <QDtls>
#include <QSharedPointer>

class ConnectionDetails {
    public:
        explicit ConnectionDetails();
        explicit ConnectionDetails(QHostAddress clientAddress, quint16 clientPort, bool tlsEnabled = false);
        ConnectionDetails(const ConnectionDetails &other); // copy constructor
        ~ConnectionDetails();
        QHostAddress getAddress() const;
        void setAddress(const QHostAddress &value);
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
        QSharedPointer<QDtls> getDtlscontext() const;
        void setDtlscontext(const QSharedPointer<QDtls> &value);
        void bumpLastTimestamp();
        QDateTime getLastPacketTimeStamp() const;
        int getTimeoutRetry() const;
        void setTimeoutRetry(int value);
        void incrTimeoutRetries();
    private:
        QHostAddress address;
        quint16 port;
        bool tlsEnabled;
        QDateTime creationTimeStamp;
        QDateTime lastPacketTimeStamp;
        int sid;
        QString hostname;
        QSharedPointer<QDtls> dtlscontext;
        int timeoutRetry;
};

class ConnectionDetailsList : public QList<QSharedPointer<ConnectionDetails> > {
        using QList::QList;
    public:
        int connectionIndex(const QHostAddress &clientAddress, const quint16 &clientPort);
};

inline uint qHash(const ConnectionDetails &key)
{
    return qHash(key.getAddress()) ^ qHash(key.getPort());
}

#endif // CONNECTIONDETAILS_H
