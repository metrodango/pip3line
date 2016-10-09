#include "sslconf.h"
#include <QFile>
#if QT_VERSION >= 0x050000
#include <QSslCertificateExtension>
#endif
#include <QSslSocket>
#include "ssloptionswidget.h"
#include <QDebug>
#include <QDateTime>

#if QT_VERSION >= 0x050400
const QList<QSsl::KeyAlgorithm> SslConf::keyAlgos = QList<QSsl::KeyAlgorithm>() << QSsl::Rsa << QSsl::Dsa << QSsl::Ec;
#else
const QList<QSsl::KeyAlgorithm> SslConf::keyAlgos = QList<QSsl::KeyAlgorithm>() << QSsl::Rsa << QSsl::Dsa;
#endif

SslConf::SslConf(QObject *parent) : QObject(parent)
{
    QSslConfiguration defConf = QSslConfiguration::defaultConfiguration();
    defConf.setPeerVerifyMode(QSslSocket::QueryPeer);
    setSslConfiguration(defConf);
    usesSystemCAs = true;
    useSNI = true;
    guiConf = nullptr;
}

SslConf::~SslConf()
{

}
QList<QSslCipher> SslConf::getSslCiphers() const
{
    return sslCiphers;
}

void SslConf::setSslCiphers(const QList<QSslCipher> &value)
{
    sslCiphers = value;
}
QSslConfiguration SslConf::getSslConfiguration()
{
    QList<QSslCertificate> CAs = otherCAs;
    if (usesSystemCAs)
        CAs.append(QSslSocket::systemCaCertificates());
    sslConfiguration.setCaCertificates(CAs);
    sslConfiguration.setCiphers(sslCiphers);
    sslConfiguration.setLocalCertificate(localCert);
    sslConfiguration.setPrivateKey(localKey);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableEmptyFragments, disableEmptyFragments);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableSessionTickets, disableSessionTickets);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableCompression, disableCompression);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableServerNameIndication, disableServerNameIndication);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableLegacyRenegotiation, disableLegacyRenegotiation);
#if QT_VERSION >= 0x050000
    sslConfiguration.setSslOption(QSsl::SslOptionDisableSessionSharing, disableSessionSharing);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableSessionPersistence, disableSessionPersistence);
#endif
    sslConfiguration.setPeerVerifyMode(sslVerificationMode);
    return sslConfiguration;
}

void SslConf::setSslConfiguration(const QSslConfiguration &value)
{
    sslConfiguration = value;
    QSslCertificate cert = sslConfiguration.localCertificate();
    if (!cert.isNull())
        localCert = cert;

    QSslKey key = sslConfiguration.privateKey();
    if (!key.isNull())
        localKey = key;

    QList<QSslCertificate> defaultCerts = QSslSocket::defaultCaCertificates();
    QList<QSslCertificate> current = sslConfiguration.caCertificates();
    for (int i = 0; i < defaultCerts.size(); i++) {
        if (current.contains(defaultCerts.at(i)))
            current.removeAll(defaultCerts.at(i));
    }

    if (!current.isEmpty()) {
        otherCAs.append(current);
    }

    sslCiphers = sslConfiguration.ciphers();

    disableEmptyFragments = sslConfiguration.testSslOption(QSsl::SslOptionDisableEmptyFragments);
    disableSessionTickets = sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionTickets);
    disableCompression = sslConfiguration.testSslOption(QSsl::SslOptionDisableCompression);
    disableServerNameIndication = sslConfiguration.testSslOption(QSsl::SslOptionDisableServerNameIndication);
    disableLegacyRenegotiation = sslConfiguration.testSslOption(QSsl::SslOptionDisableLegacyRenegotiation);
#if QT_VERSION >= 0x050000
    disableSessionSharing = sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionSharing);
    disableSessionPersistence = sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionPersistence);
#endif

    sslVerificationMode = sslConfiguration.peerVerifyMode();
}

void SslConf::clearOtherCAs()
{
    if (!otherCAs.isEmpty()) {
        otherCAs.clear();
    }
}
QString SslConf::getSslPeerNameSNI() const
{
    return sslPeerNameSNI;
}

void SslConf::setSslPeerNameSNI(const QString &value)
{
    if (sslPeerNameSNI != value) {
        sslPeerNameSNI = value;
    }
}

bool SslConf::isUsingSNI() const
{
    return useSNI;
}

void SslConf::setUseSNI(bool value)
{
    useSNI = value;
}

void SslConf::onGuiDeleted()
{
    guiConf = nullptr;
}

QString SslConf::concat(const QStringList &list)
{
    QString temp;
    for (int i = 0; i < list.size(); i++) {
        temp.append(list.at(i)).append("\n");
    }
    temp.chop(1);
    return temp;
}

QSslSocket::PeerVerifyMode SslConf::getSslVerificationMode() const
{
    return sslVerificationMode;
}

void SslConf::setSslVerificationMode(const QSslSocket::PeerVerifyMode &value)
{
    sslVerificationMode = value;
}

QWidget *SslConf::getGui(QWidget *parent)
{
    if (guiConf == nullptr) {
        guiConf = new(std::nothrow) SSLOptionsWidget(getSslConfiguration(),useSNI,sslPeerNameSNI, usesSystemCAs, parent);
        if (guiConf == nullptr) {
            qFatal("Cannot allocate memory for SSLOptionsWidget X{");
        }

        connect(guiConf, SIGNAL(caListUpdated(QList<QSslCertificate>)), SLOT(setOtherCAs(QList<QSslCertificate>)));
        connect(guiConf, SIGNAL(disableCompression(bool)), SLOT(setDisableCompression(bool)));
        connect(guiConf, SIGNAL(disableEmptyFragments(bool)), SLOT(setDisableEmptyFragments(bool)));
        connect(guiConf, SIGNAL(disableLegacyRenegotiation(bool)), SLOT(setDisableLegacyRenegotiation(bool)));
        connect(guiConf, SIGNAL(disableServerNameIndication(bool)), SLOT(setDisableServerNameIndication(bool)));
#if QT_VERSION >= 0x050000
        connect(guiConf, SIGNAL(disableSessionPersistence(bool)), SLOT(setDisableSessionPersistence(bool)));
        connect(guiConf, SIGNAL(disableSessionSharing(bool)), SLOT(setDisableSessionSharing(bool)));
#endif
        connect(guiConf, SIGNAL(disableSessionTickets(bool)), SLOT(setDisableSessionTickets(bool)));
        connect(guiConf, SIGNAL(sslCiphersUpdated(QList<QSslCipher>)), SLOT(setSslCiphers(QList<QSslCipher>)));
        connect(guiConf, SIGNAL(sslLocalCertUpdated(QSslCertificate)), SLOT(setLocalCert(QSslCertificate)));
        connect(guiConf, SIGNAL(sslLocalKeyUpdated(QSslKey)), SLOT(setLocalKey(QSslKey)));
        connect(guiConf, SIGNAL(useOfSystemCAUpdated(bool)), SLOT(setUseSystemCAs(bool)));
        connect(guiConf, SIGNAL(peerVerificationModeChanged(QSslSocket::PeerVerifyMode)), SLOT(setSslVerificationMode(QSslSocket::PeerVerifyMode)));
        connect(guiConf, SIGNAL(peerVerificationNameChanged(QString)), SLOT(setSslPeerNameSNI(QString)));
        connect(guiConf, SIGNAL(sniEnabled(bool)), SLOT(setUseSNI(bool)));
        connect(guiConf, SIGNAL(destroyed(QObject*)), SLOT(onGuiDeleted()));
    }

    return guiConf;
}

#if QT_VERSION >= 0x050000

bool SslConf::getDisableSessionPersistence() const
{
    return disableSessionPersistence;
}

void SslConf::setDisableSessionPersistence(bool value)
{
    if (disableSessionPersistence != value) {
        disableSessionPersistence = value;
    }
}

bool SslConf::getDisableSessionSharing() const
{
    return disableSessionSharing;
}

void SslConf::setDisableSessionSharing(bool value)
{
    if (disableSessionSharing != value) {
        disableSessionSharing = value;
    }
}
#endif

bool SslConf::getDisableLegacyRenegotiation() const
{
    return disableLegacyRenegotiation;
}

void SslConf::setDisableLegacyRenegotiation(bool value)
{
    if (disableLegacyRenegotiation != value) {
        disableLegacyRenegotiation = value;
    }
}

bool SslConf::getDisableServerNameIndication() const
{
    return disableServerNameIndication;
}

void SslConf::setDisableServerNameIndication(bool value)
{
    if (disableServerNameIndication != value) {
        disableServerNameIndication = value;
    }
}

bool SslConf::getDisableCompression() const
{
    return disableCompression;
}

void SslConf::setDisableCompression(bool value)
{
    disableCompression = value;
}

bool SslConf::getDisableSessionTickets() const
{
    return disableSessionTickets;
}

void SslConf::setDisableSessionTickets(bool value)
{
        disableSessionTickets = value;
}

bool SslConf::getDisableEmptyFragments() const
{
    return disableEmptyFragments;
}

void SslConf::setDisableEmptyFragments(bool value)
{
        disableEmptyFragments = value;
}

bool SslConf::isUsingSystemCAs() const
{
    return usesSystemCAs;
}

void SslConf::setUseSystemCAs(bool value)
{
        usesSystemCAs = value;
}


QList<QSslCertificate> SslConf::getOtherCAs() const
{
    return otherCAs;
}

void SslConf::setOtherCAs(const QList<QSslCertificate> &value)
{
        otherCAs = value;
}

QSslCertificate SslConf::getLocalCert() const
{
    return localCert;
}

void SslConf::setLocalCert(const QSslCertificate &value)
{
    if (localCert != value && !value.isNull()) {
        qDebug() << "Cert loaded ";
        localCert = value;
    }
}

void SslConf::setLocalCert(const QString filename)
{
    qDebug() << "Loading cert from " << filename;
    if (!filename.isEmpty()) {
        QFile certFile(filename);
        QSslCertificate localCert;
        if (certFile.open(QIODevice::ReadOnly)) {
            QList<QSslCertificate> list = QSslCertificate::fromDevice(&certFile, QSsl::Pem);
            if (!list.isEmpty()) {
                if (list.size() > 1)
                    emit log(tr("Multiple certificates loaded, considering only the first one"), metaObject()->className(), Pip3lineConst::LWARNING);

                localCert = list.at(0);
#if QT_VERSION >= 0x050000
                emit log(tr("Local Cert loaded : %1").arg(concat(localCert.subjectInfo(QSslCertificate::CommonName))), metaObject()->className(), Pip3lineConst::LSTATUS);
                emit log(tr("== Issuer: %1").arg(concat(localCert.issuerInfo(QSslCertificate::CommonName))), metaObject()->className(), Pip3lineConst::LSTATUS);
#else
                emit log(tr("Local Cert loaded : %1").arg(localCert.subjectInfo(QSslCertificate::CommonName)), metaObject()->className(), Pip3lineConst::LSTATUS);
                emit log(tr("== Issuer: %1").arg(localCert.issuerInfo(QSslCertificate::CommonName)), metaObject()->className(), Pip3lineConst::LSTATUS);
#endif

                emit log(tr("   From  :%1").arg(localCert.effectiveDate().toString()), metaObject()->className(), Pip3lineConst::LSTATUS);
                emit log(tr("   Until :%1").arg(localCert.expiryDate().toString()), metaObject()->className(), Pip3lineConst::LSTATUS);

#if QT_VERSION >= 0x050000
                QList<QSslCertificateExtension> list = localCert.extensions();
                emit log(tr("   Extensions :%1").arg(list.size()), metaObject()->className(), Pip3lineConst::LSTATUS);
                for (int i = 0; i < list.size(); i++) {
                    QSslCertificateExtension ext = list.at(i);
                    QVariant val = ext.value();
                    switch (val.type()) {
                        case QVariant::String:
                            emit log(tr("    %1 %2").arg(ext.name()).arg(val.toString()), metaObject()->className(), Pip3lineConst::LSTATUS);
                            break;
                        case QVariant::Map:
                            {
                                emit log(tr("    %1").arg(ext.name()), metaObject()->className(), Pip3lineConst::LSTATUS);

                                QVariantMap map = val.toMap();
                                QMapIterator<QString, QVariant> i(map);
                                while (i.hasNext()) {
                                    i.next();
                                    emit log(tr("    %1 => %2").arg(i.key()).arg(i.value().toString()), metaObject()->className(), Pip3lineConst::LSTATUS);
                                }
                            }
                            break;
                        default:
                            emit log(tr("    %1 %2").arg(ext.name()).arg(val.type()), metaObject()->className(), Pip3lineConst::LSTATUS);
                    }
                }
#endif
                emit log(tr("   %1").arg(QString::fromUtf8(localCert.digest(QCryptographicHash::Sha1).toHex())), metaObject()->className(), Pip3lineConst::LSTATUS);
#if QT_VERSION >= 0x050000
                emit log(tr("   %1").arg(QString::fromUtf8(localCert.digest(QCryptographicHash::Sha256).toHex())), metaObject()->className(), Pip3lineConst::LSTATUS);
#endif
                setLocalCert(localCert);
            } else {
                emit log(tr("No server local loaded [%1]").arg(certFile.fileName()), metaObject()->className(), Pip3lineConst::LERROR);
            }
        } else {
            emit log(tr("Cannot open the local cert: %1").arg(certFile.fileName()), metaObject()->className(), Pip3lineConst::LERROR);
        }
    }
}

QSslKey SslConf::getLocalKey() const
{
    return localKey;
}

void SslConf::setLocalKey(const QSslKey &value)
{
    if (localKey != value) {
        if (value.isNull())
            emit log(tr("Trying to set a null key, ignoring T_T"), metaObject()->className(), Pip3lineConst::LERROR);
        else {
            localKey = value;
        }
    }
}

void SslConf::setLocalKey(const QString filename)
{
    qDebug() << "Loading key from " << filename;
    QFile keyFile(filename);
    QSslKey localKey;
    if (keyFile.open(QIODevice::ReadOnly)) {
        for (int j = 0; j < keyAlgos.size(); ++j) {
            localKey = QSslKey(&keyFile,keyAlgos.at(j),QSsl::Pem,QSsl::PrivateKey);
            if (!localKey.isNull()) {
                emit log(tr("  => Got private key :->"), metaObject()->className(), Pip3lineConst::LSTATUS);
                break;
            }
        }

        if (localKey.isNull()) {
            emit log(tr("No local Key loaded [%1]").arg(keyFile.fileName()), metaObject()->className(), Pip3lineConst::LERROR);
        } else {
            setLocalKey(localKey);
        }
    } else {
        emit log(tr("Cannot open the local key file: %1").arg(keyFile.fileName()), metaObject()->className(), Pip3lineConst::LERROR);
    }
}

QString SslConf::sslModeToString(int mode)
{
    QString ret;
    switch (mode) {
        case 0:
            ret = QString("UnencryptedMode");
            break;
        case 1:
            ret = QString("SslClientMode");
            break;
        case 2:
            ret = QString("SslServerMode");
            break;
        default:
            ret = QString("Unknown mode: %1").arg(mode);
    }

    return ret;
}



