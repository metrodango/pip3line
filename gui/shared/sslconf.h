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
        static const QString CONF_SSL_IS_USING_SYSTEM_CAS;
        static const QString CONF_SSL_DISABLE_EMPTY_FRAGMENT;
        static const QString CONF_SSL_DISABLE_SESSION_TICKETS;
        static const QString CONF_SSL_DISABLE_COMPRESSION;
        static const QString CONF_SSL_ENABLE_SERVER_NAME_INDICATION;
        static const QString CONF_SSL_DISABLE_LEGACY_RENEGOTIATION;
        static const QString CONF_SSL_DISABLE_SESSION_SHARING;
        static const QString CONF_SSL_DISABLE_SESSION_PERSISTENCE;
        static const QString CONF_SSL_SNI_VALUE;
        static const QString CONF_SSL_OTHER_CAS;
        static const QString CONF_SSL_CIPHERS;
        static const QString CONF_SSL_LOCAL_PRIVATE_KEY;
        static const QString CONF_SSL_LOCAL_CERTIFICATE;
        static const QString CONF_SSL_ALLOWED_NEXT_PROTOCOLS;
        static const QString CONF_SSL_ELLIPTICS_CURVES;

        explicit SslConf(QObject *parent = nullptr);
        ~SslConf();
        QList<QSslCipher> getSslCiphers() const;
        QSslKey getLocalKey() const;
        QSslCertificate getLocalCert() const;
        QList<QSslCertificate> getOtherCAs() const;

        bool isUsingSystemCAs() const;
        bool getDisableEmptyFragments() const;
        bool getDisableSessionTickets() const;
        bool getDisableCompression() const;
        bool getDisableLegacyRenegotiation() const;
        bool getDisableSessionSharing() const;
        bool getDisableSessionPersistence() const;
        QWidget *getGui(QWidget *parent = nullptr);
        QSslSocket::PeerVerifyMode getSslVerificationMode() const;
        QString getSslPeerNameSNI() const;
        bool isUsingSNI() const;
        QSslConfiguration getSslConfiguration();
        virtual QHash<QString, QString> getConfiguration();
        virtual void setConfiguration(QHash<QString, QString> conf);
        static QString sslModeToString(int mode);
        QList<QByteArray> getAllowedNextProtocols() const;
        QVector<QSslEllipticCurve> getEllipticCurves() const;
        QByteArray getPsk() const;
        void setPsk(const QByteArray &value);
        void setProtocol(QSsl::SslProtocol proto);
    signals:
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
    public slots:
        void setDisableEmptyFragments(bool value);
        void setDisableSessionTickets(bool value);
        void setDisableCompression(bool value);
        void setDisableLegacyRenegotiation(bool value);
        void setDisableSessionSharing(bool value);
        void setDisableSessionPersistence(bool value);
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
        void setAllowedNextProtocols(const QList<QByteArray> &value);
        void setEllipticCurves(const QVector<QSslEllipticCurve> &value);
    private slots:
        void onGuiDeleted();
    private:
        Q_DISABLE_COPY(SslConf)
        static const QList<QSsl::KeyAlgorithm> keyAlgos;
        static const QString CONF_SSL_CIPHER_SEPARATOR;
        QString concat(const QStringList &list);
        bool disableEmptyFragments;
        bool disableSessionTickets;
        bool disableCompression;
        bool disableLegacyRenegotiation;
        bool disableSessionSharing;
        bool disableSessionPersistence;
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
        QList<QByteArray> allowedNextProtocols;
        QVector<QSslEllipticCurve> ellipticCurves;
        QByteArray psk;
        QSsl::SslProtocol protocolVersion;
};

#endif // SSLCONF_H
