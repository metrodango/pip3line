#ifndef SSLCONF_H
#define SSLCONF_H

#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <QList>
#include <QSslCipher>
#include <QSslSocket>
#include <QWidget>
#include <commonstrings.h>

class SSLOptionsWidget;

class SslConf : public QObject
{
        Q_OBJECT
    public:
        explicit SslConf(QObject *parent = 0);
        ~SslConf();
        QList<QSslCipher> getSslCiphers() const;
        QSslKey getLocalKey() const;
        QSslCertificate getLocalCert() const;
        QList<QSslCertificate> getOtherCAs() const;
        QSslConfiguration getSslConfiguration();
        bool isUsingSystemCAs() const;
        bool getDisableEmptyFragments() const;
        bool getDisableSessionTickets() const;
        bool getDisableCompression() const;
        bool getDisableServerNameIndication() const;
        bool getDisableLegacyRenegotiation() const;
#if QT_VERSION >= 0x050000
        bool getDisableSessionSharing() const;
        bool getDisableSessionPersistence() const;
#endif
        QWidget *getGui(QWidget *parent = 0);
        QSslSocket::PeerVerifyMode getSslVerificationMode() const;
        QString getSslPeerNameSNI() const;
        bool isUsingSNI() const;
        static QString sslModeToString(int mode);
    signals:
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
    public slots:
        void setDisableEmptyFragments(bool value);
        void setDisableSessionTickets(bool value);
        void setDisableCompression(bool value);
        void setDisableServerNameIndication(bool value);
        void setDisableLegacyRenegotiation(bool value);
#if QT_VERSION >= 0x050000
        void setDisableSessionSharing(bool value);
        void setDisableSessionPersistence(bool value);
#endif
        void setUseSystemCAs(bool value);
        void setOtherCAs(const QList<QSslCertificate> &value);
        void setLocalCert(const QSslCertificate &value);
        void setLocalCert(const QString filename);
        void setLocalKey(const QSslKey &value);
        void setLocalKey(const QString filename);
        void setSslCiphers(const QList<QSslCipher> &value);
        void setSslConfiguration(const QSslConfiguration &value);
        void setSslVerificationMode(const QSslSocket::PeerVerifyMode &value);
        void clearOtherCAs();
        void setSslPeerNameSNI(const QString &value);
        void setUseSNI(bool value);
    private slots:
        void onGuiDeleted();
    private:
        static const QList<QSsl::KeyAlgorithm> keyAlgos;
        QString concat(const QStringList &list);
        bool disableEmptyFragments;
        bool disableSessionTickets;
        bool disableCompression;
        bool disableServerNameIndication;
        bool disableLegacyRenegotiation;
#if QT_VERSION >= 0x050000
        bool disableSessionSharing;
        bool disableSessionPersistence;
#endif
        QSslSocket::PeerVerifyMode sslVerificationMode;
        bool usesSystemCAs;
        QSslConfiguration sslConfiguration;
        QList<QSslCertificate> otherCAs;
        QSslCertificate localCert;
        QSslKey localKey;
        QList<QSslCipher> sslCiphers;
        SSLOptionsWidget *guiConf;
        QString sslPeerNameSNI;
        bool useSNI;
};

#endif // SSLCONF_H
