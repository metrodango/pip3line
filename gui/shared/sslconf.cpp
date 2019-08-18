#include "sslconf.h"
#include <QFile>
#include <QSslCertificateExtension>
#include <QSslSocket>
#include "ssloptionswidget.h"
#include <QDebug>
#include <QDateTime>
#include "shared/guiconst.h"
#include <QSslEllipticCurve>

#if QT_VERSION >= 0x050400
const QList<QSsl::KeyAlgorithm> SslConf::keyAlgos = QList<QSsl::KeyAlgorithm>() << QSsl::Rsa << QSsl::Dsa << QSsl::Ec;
#else
const QList<QSsl::KeyAlgorithm> SslConf::keyAlgos = QList<QSsl::KeyAlgorithm>() << QSsl::Rsa << QSsl::Dsa;
#endif

const QString SslConf::CONF_SSL_IS_USING_SYSTEM_CAS = "SSL_IsUsingSystemCAs";
const QString SslConf::CONF_SSL_DISABLE_EMPTY_FRAGMENT = "SSL_DisableEmptyFragments";
const QString SslConf::CONF_SSL_DISABLE_SESSION_TICKETS = "SSL_DisableSessionTickets";
const QString SslConf::CONF_SSL_DISABLE_COMPRESSION = "SSL_DisableCompression";
const QString SslConf::CONF_SSL_ENABLE_SERVER_NAME_INDICATION = "SSL_EnableSNI";
const QString SslConf::CONF_SSL_DISABLE_LEGACY_RENEGOTIATION = "SSL_DisableLegacyRenegotiation";
const QString SslConf::CONF_SSL_DISABLE_SESSION_SHARING = "SSL_DisableSessionSharing";
const QString SslConf::CONF_SSL_DISABLE_SESSION_PERSISTENCE = "SSL_DisableSessionPersistence";
const QString SslConf::CONF_SSL_SNI_VALUE = "SSL_SNIValue";
const QString SslConf::CONF_SSL_OTHER_CAS = "SSL_OtherCAs";
const QString SslConf::CONF_SSL_CIPHERS = "SSL_Ciphers";
const QString SslConf::CONF_SSL_LOCAL_PRIVATE_KEY = "SSL_LocalPrivateKey";
const QString SslConf::CONF_SSL_LOCAL_CERTIFICATE = "SSL_LocalCertificate";
const QString SslConf::CONF_SSL_ALLOWED_NEXT_PROTOCOLS = "SSL_AllowedNextProtocols";
const QString SslConf::CONF_SSL_ELLIPTICS_CURVES = "SSL_EllipticCurves";
const QString SslConf::CONF_SSL_CIPHER_SEPARATOR = ";";

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
    delete guiConf;
}

QList<QSslCipher> SslConf::getSslCiphers() const
{
    return sslCiphers;
}

void SslConf::setSslCiphers(const QList<QSslCipher> &value)
{
    qDebug() << tr("Enabled Cipher list updated %1").arg(value.size());
    sslCiphers = value;
}

QSslConfiguration SslConf::getSslConfiguration()
{
    QList<QSslCertificate> CAs = otherCAs;
    if (usesSystemCAs) {
        CAs.append(QSslConfiguration::systemCaCertificates());
    }
    sslConfiguration.setCaCertificates(CAs);
    sslConfiguration.setCiphers(sslCiphers);
    sslConfiguration.setLocalCertificate(localCert);
    sslConfiguration.setPrivateKey(localKey);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableEmptyFragments, disableEmptyFragments);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableSessionTickets, disableSessionTickets);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableCompression, disableCompression);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableServerNameIndication, !useSNI);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableLegacyRenegotiation, disableLegacyRenegotiation);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableSessionSharing, disableSessionSharing);
    sslConfiguration.setSslOption(QSsl::SslOptionDisableSessionPersistence, disableSessionPersistence);
    sslConfiguration.setPeerVerifyMode(sslVerificationMode);
    sslConfiguration.setAllowedNextProtocols(allowedNextProtocols);
    sslConfiguration.setEllipticCurves(ellipticCurves);
    return sslConfiguration;
}

QHash<QString, QString> SslConf::getConfiguration()
{
    QHash<QString, QString> conf;
    conf.insert(CONF_SSL_IS_USING_SYSTEM_CAS, QString::number(usesSystemCAs ? 1 : 0));
    conf.insert(CONF_SSL_DISABLE_EMPTY_FRAGMENT, QString::number(disableEmptyFragments ? 1 : 0));
    conf.insert(CONF_SSL_DISABLE_SESSION_TICKETS, QString::number(disableSessionTickets ? 1 : 0));
    conf.insert(CONF_SSL_DISABLE_COMPRESSION, QString::number(disableCompression ? 1 : 0));
    conf.insert(CONF_SSL_ENABLE_SERVER_NAME_INDICATION, QString::number(useSNI ? 1 : 0));
    conf.insert(CONF_SSL_DISABLE_LEGACY_RENEGOTIATION, QString::number(disableLegacyRenegotiation ? 1 : 0));
    conf.insert(CONF_SSL_DISABLE_SESSION_SHARING, QString::number(disableSessionSharing ? 1 : 0));
    conf.insert(CONF_SSL_DISABLE_SESSION_PERSISTENCE, QString::number(disableSessionPersistence ? 1 : 0));
    conf.insert(CONF_SSL_SNI_VALUE, sslPeerNameSNI);
    QString otherCasSTRList;
    for (int i = 0; i < otherCAs.size(); i++) {
        QByteArray cert = otherCAs.at(i).toPem();
        otherCasSTRList.append(QString::fromUtf8(cert));
    }
    if (!otherCasSTRList.isEmpty())
        conf.insert(CONF_SSL_OTHER_CAS, otherCasSTRList);
    QString cipherSTRList;
    for (int i = 0; i < sslCiphers.size(); i++) {
        QSslCipher ciph = sslCiphers.at(i);
        cipherSTRList.append(QString::number(ciph.protocol()))
                .append(GuiConst::STATE_FIELD_SEPARATOR)
                .append(ciph.name())
                .append(CONF_SSL_CIPHER_SEPARATOR);
    }
    if (!cipherSTRList.isEmpty())
        conf.insert(CONF_SSL_CIPHERS, cipherSTRList);
    QString privValue = QString::number(static_cast<int>(localKey.algorithm()));
    privValue.append(GuiConst::STATE_FIELD_SEPARATOR);
    privValue.append(QString::fromUtf8(localKey.toPem()));
    conf.insert(CONF_SSL_LOCAL_PRIVATE_KEY, privValue);

    QString localCertSTR = QString::fromUtf8(localCert.toPem());
    conf.insert(CONF_SSL_LOCAL_CERTIFICATE, localCertSTR);

    QString allowedNextSTR;
    for (int i = 0; i < allowedNextProtocols.size(); i++) {
        allowedNextSTR.append(QString::fromUtf8(allowedNextProtocols.at(i).toHex()))
                .append(GuiConst::STATE_FIELD_SEPARATOR);
    }
    if (!allowedNextSTR.isEmpty())
        conf.insert(CONF_SSL_ALLOWED_NEXT_PROTOCOLS, allowedNextSTR);

    QString ellipticCurvesSTR;
    for (int i = 0; i < ellipticCurves.size(); i++) {
        ellipticCurvesSTR.append(ellipticCurves.at(i).shortName())
                .append(GuiConst::STATE_FIELD_SEPARATOR);
    }
    if (!ellipticCurvesSTR.isEmpty())
        conf.insert(CONF_SSL_ELLIPTICS_CURVES, ellipticCurvesSTR);

    return conf;
}

void SslConf::setConfiguration(QHash<QString, QString> conf)
{
    bool ok = false;
    if (conf.contains(CONF_SSL_IS_USING_SYSTEM_CAS)) {
        int val = conf.value(CONF_SSL_IS_USING_SYSTEM_CAS).toInt(&ok);
        if (ok && (val == 1 || val == 0)) {
            usesSystemCAs = (val == 1);
        }
    }

    if (conf.contains(CONF_SSL_DISABLE_EMPTY_FRAGMENT)) {
        int val = conf.value(CONF_SSL_DISABLE_EMPTY_FRAGMENT).toInt(&ok);
        if (ok && (val == 1 || val == 0)) {
            disableEmptyFragments = (val == 1);
        }
    }

    if (conf.contains(CONF_SSL_DISABLE_SESSION_TICKETS)) {
        int val = conf.value(CONF_SSL_DISABLE_SESSION_TICKETS).toInt(&ok);
        if (ok && (val == 1 || val == 0)) {
            disableSessionTickets = (val == 1);
        }
    }

    if (conf.contains(CONF_SSL_DISABLE_COMPRESSION)) {
        int val = conf.value(CONF_SSL_DISABLE_COMPRESSION).toInt(&ok);
        if (ok && (val == 1 || val == 0)) {
            disableCompression = (val == 1);
        }
    }

    if (conf.contains(CONF_SSL_ENABLE_SERVER_NAME_INDICATION)) {
        int val = conf.value(CONF_SSL_ENABLE_SERVER_NAME_INDICATION).toInt(&ok);
        if (ok && (val == 1 || val == 0)) {
            useSNI = (val == 1);
        }
    }

    if (conf.contains(CONF_SSL_DISABLE_LEGACY_RENEGOTIATION)) {
        int val = conf.value(CONF_SSL_DISABLE_LEGACY_RENEGOTIATION).toInt(&ok);
        if (ok && (val == 1 || val == 0)) {
            disableLegacyRenegotiation = (val == 1);
        }
    }

    if (conf.contains(CONF_SSL_DISABLE_SESSION_SHARING)) {
        int val = conf.value(CONF_SSL_DISABLE_SESSION_SHARING).toInt(&ok);
        if (ok && (val == 1 || val == 0)) {
            disableSessionSharing = (val == 1);
        }
    }

    if (conf.contains(CONF_SSL_DISABLE_SESSION_PERSISTENCE)) {
        int val = conf.value(CONF_SSL_DISABLE_SESSION_PERSISTENCE).toInt(&ok);
        if (ok && (val == 1 || val == 0)) {
            disableSessionPersistence = (val == 1);
        }
    }

    if (conf.contains(CONF_SSL_SNI_VALUE)) {
        sslPeerNameSNI = conf.value(CONF_SSL_SNI_VALUE);
    }

    if (conf.contains(CONF_SSL_OTHER_CAS)) {
        otherCAs = QSslCertificate::fromData(conf.value(CONF_SSL_OTHER_CAS).toUtf8(),QSsl::Pem);
    }

    if (conf.contains(CONF_SSL_CIPHERS)) {
        sslCiphers.clear();
        QString ciphers_str = conf.value(CONF_SSL_CIPHERS);
        QStringList cipherListstr = ciphers_str.split(CONF_SSL_CIPHER_SEPARATOR, QString::SkipEmptyParts);
        for (int i = 0; i < cipherListstr.size(); i++) {
            QStringList compoCipher = cipherListstr.at(i).split(GuiConst::STATE_FIELD_SEPARATOR, QString::SkipEmptyParts);
            if (compoCipher.size() == 2) {
                int protoVal = compoCipher.at(0).toInt(&ok);
                if (ok && (protoVal == QSsl::SslV2 ||
                           protoVal == QSsl::SslV3 ||
                           protoVal == QSsl::TlsV1_0 ||
                           protoVal == QSsl::TlsV1_1 ||
                           protoVal == QSsl::TlsV1_2)) {
                    QSslCipher ciph = QSslCipher(compoCipher.at(1), static_cast<QSsl::SslProtocol>(protoVal));
                    if (!ciph.isNull()) {
                        sslCiphers.append(ciph);
                    } else {
                        qDebug() << tr("[SslConf::setConfiguratio] resulting cipher is null");
                    }
                }
            } // else just ignore
        }

        qDebug() << tr("Cipher list loaded %1").arg(sslCiphers.size());
    }

    if (conf.contains(CONF_SSL_LOCAL_PRIVATE_KEY)) {
        QString valraw = conf.value(CONF_SSL_LOCAL_PRIVATE_KEY);
        int index = valraw.indexOf(GuiConst::STATE_FIELD_SEPARATOR);
        if (index == 1) {
            int algoval = valraw.left(1).toInt(&ok);
            if (ok && (algoval == QSsl::Rsa || algoval == QSsl::Dsa || algoval == QSsl::Ec)) {
                localKey = QSslKey(valraw.mid(2).toUtf8(), static_cast<QSsl::KeyAlgorithm>(algoval), QSsl::Pem);
                qDebug() << tr("Private key loaded");
            }
        }
    }

    if (conf.contains(CONF_SSL_LOCAL_CERTIFICATE)) {
        QList<QSslCertificate> list = QSslCertificate::fromData(conf.value(CONF_SSL_LOCAL_CERTIFICATE).toUtf8(),QSsl::Pem);
        if (list.size() > 0)  {
            localCert = list.at(0);// ignoring any other cert loaded
            qDebug() << tr("Certificate loaded");
        } else {
            qDebug() << tr("No certificate loaded");
        }
    }

    if (conf.contains(CONF_SSL_ALLOWED_NEXT_PROTOCOLS)) {
        allowedNextProtocols.clear();
        QStringList allowedNextList = conf.value(CONF_SSL_ALLOWED_NEXT_PROTOCOLS).split(GuiConst::STATE_FIELD_SEPARATOR, QString::SkipEmptyParts);
        for (int i = 0; i < allowedNextList.size(); i++) {
            QByteArray val = QByteArray::fromHex(allowedNextList.at(i).toUtf8());
            if (!val.isEmpty()) {
                allowedNextProtocols.append(val);
            }
        }
    }

    if (conf.contains(CONF_SSL_ELLIPTICS_CURVES)) {
        ellipticCurves.clear();
        QStringList ellipticCurvesList = conf.value(CONF_SSL_ELLIPTICS_CURVES).split(GuiConst::STATE_FIELD_SEPARATOR, QString::SkipEmptyParts);
        for (int i = 0; i < ellipticCurvesList.size(); i++) {
            QSslEllipticCurve curve = QSslEllipticCurve::fromShortName(ellipticCurvesList.at(i));
            if (curve.isValid()) {
                ellipticCurves.append(curve);
            }
        }
        qDebug() << tr("Elliptic curves loaded: %1").arg(ellipticCurves.size());
    }
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

    //QSslConfiguration defConf = QSslConfiguration::defaultConfiguration();

    QList<QSslCertificate> defaultCerts = QSslConfiguration::systemCaCertificates();
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
    useSNI = !sslConfiguration.testSslOption(QSsl::SslOptionDisableServerNameIndication);
    disableLegacyRenegotiation = sslConfiguration.testSslOption(QSsl::SslOptionDisableLegacyRenegotiation);
    disableSessionSharing = sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionSharing);
    disableSessionPersistence = sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionPersistence);

    sslVerificationMode = sslConfiguration.peerVerifyMode();
    allowedNextProtocols = sslConfiguration.allowedNextProtocols();
    ellipticCurves = sslConfiguration.ellipticCurves();
}

void SslConf::clearOtherCAs()
{
    if (!otherCAs.isEmpty()) {
        qDebug() << tr("Clear all other CAs");
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

QVector<QSslEllipticCurve> SslConf::getEllipticCurves() const
{
    return ellipticCurves;
}

void SslConf::setEllipticCurves(const QVector<QSslEllipticCurve> &value)
{
    ellipticCurves = value;
}

QList<QByteArray> SslConf::getAllowedNextProtocols() const
{
    return allowedNextProtocols;
}

void SslConf::setAllowedNextProtocols(const QList<QByteArray> &value)
{
    allowedNextProtocols = value;
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
        guiConf = new(std::nothrow) SSLOptionsWidget(getSslConfiguration(),
                                                     useSNI,
                                                     sslPeerNameSNI,
                                                     usesSystemCAs,
                                                     otherCAs,
                                                     parent);
        if (guiConf == nullptr) {
            qFatal("Cannot allocate memory for SSLOptionsWidget X{");
        }

        connect(guiConf, &SSLOptionsWidget::caListUpdated, this, &SslConf::setOtherCAs);
        connect(guiConf, &SSLOptionsWidget::disableCompression, this, &SslConf::setDisableCompression);
        connect(guiConf, &SSLOptionsWidget::disableEmptyFragments, this, &SslConf::setDisableEmptyFragments);
        connect(guiConf, &SSLOptionsWidget::disableLegacyRenegotiation, this, &SslConf::setDisableLegacyRenegotiation);
        connect(guiConf, &SSLOptionsWidget::disableSessionPersistence, this, &SslConf::setDisableSessionPersistence);
        connect(guiConf, &SSLOptionsWidget::disableSessionSharing, this, &SslConf::setDisableSessionSharing);
        connect(guiConf, &SSLOptionsWidget::disableSessionTickets, this, &SslConf::setDisableSessionTickets);
        connect(guiConf, &SSLOptionsWidget::sslCiphersUpdated, this, &SslConf::setSslCiphers);
        //connect(guiConf, &SSLOptionsWidget::sslLocalCertUpdated, this, qOverload<const QSslCertificate &>(&SslConf::setLocalCert));
        connect(guiConf, SIGNAL(sslLocalCertUpdated(QSslCertificate)), this, SLOT(setLocalCert(QSslCertificate)));
        //connect(guiConf, &SSLOptionsWidget::sslLocalKeyUpdated, this, qOverload<const QSslKey &>(&SslConf::setLocalKey));
        connect(guiConf, SIGNAL(sslLocalKeyUpdated(QSslKey)), this, SLOT(setLocalKey(QSslKey)));
        connect(guiConf, &SSLOptionsWidget::useOfSystemCAUpdated, this, &SslConf::setUseSystemCAs);
        connect(guiConf, &SSLOptionsWidget::peerVerificationModeChanged, this, &SslConf::setSslVerificationMode);
        connect(guiConf, &SSLOptionsWidget::peerVerificationNameChanged, this, &SslConf::setSslPeerNameSNI);
        connect(guiConf, &SSLOptionsWidget::sniEnabled, this, &SslConf::setUseSNI);
        connect(guiConf, &SSLOptionsWidget::allowedNextProtocolsUpdated, this, &SslConf::setAllowedNextProtocols);
        connect(guiConf, &SSLOptionsWidget::selectedCurvesListUpdated, this , &SslConf::setEllipticCurves);
        connect(guiConf, &SSLOptionsWidget::destroyed, [=](QObject *) { onGuiDeleted();});
    }

    return guiConf;
}

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
    qDebug() << tr("adding other CAs: %1").arg(value.size());
    otherCAs = value;
}

QSslCertificate SslConf::getLocalCert() const
{
    return localCert;
}

void SslConf::setLocalCert(const QSslCertificate &value)
{
    if (localCert != value && !value.isNull()) {
        qDebug() << "Local Cert updated ";
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

                emit log(tr("Local Cert loaded : %1").arg(concat(localCert.subjectInfo(QSslCertificate::CommonName))), metaObject()->className(), Pip3lineConst::PLSTATUS);
                emit log(tr("== Issuer: %1").arg(concat(localCert.issuerInfo(QSslCertificate::CommonName))), metaObject()->className(), Pip3lineConst::PLSTATUS);

                emit log(tr("   From  :%1").arg(localCert.effectiveDate().toString()), metaObject()->className(), Pip3lineConst::PLSTATUS);
                emit log(tr("   Until :%1").arg(localCert.expiryDate().toString()), metaObject()->className(), Pip3lineConst::PLSTATUS);

                QList<QSslCertificateExtension> list = localCert.extensions();
                emit log(tr("   Extensions :%1").arg(list.size()), metaObject()->className(), Pip3lineConst::PLSTATUS);
                for (int i = 0; i < list.size(); i++) {
                    QSslCertificateExtension ext = list.at(i);
                    QVariant val = ext.value();
                    switch (val.type()) {
                        case QVariant::String:
                            emit log(tr("    %1 %2").arg(ext.name()).arg(val.toString()), metaObject()->className(), Pip3lineConst::PLSTATUS);
                            break;
                        case QVariant::Map:
                            {
                                emit log(tr("    %1").arg(ext.name()), metaObject()->className(), Pip3lineConst::PLSTATUS);

                                QVariantMap map = val.toMap();
                                QMapIterator<QString, QVariant> i(map);
                                while (i.hasNext()) {
                                    i.next();
                                    emit log(tr("    %1 => %2").arg(i.key()).arg(i.value().toString()), metaObject()->className(), Pip3lineConst::PLSTATUS);
                                }
                            }
                            break;
                        default:
                            emit log(tr("    %1 %2").arg(ext.name()).arg(val.type()), metaObject()->className(), Pip3lineConst::PLSTATUS);
                    }
                }

                emit log(tr("   %1").arg(QString::fromUtf8(localCert.digest(QCryptographicHash::Sha1).toHex())), metaObject()->className(), Pip3lineConst::PLSTATUS);

                emit log(tr("   %1").arg(QString::fromUtf8(localCert.digest(QCryptographicHash::Sha256).toHex())), metaObject()->className(), Pip3lineConst::PLSTATUS);

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
            qDebug() << "Local private key updated ";
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
                emit log(tr("  => Got private key :->"), metaObject()->className(), Pip3lineConst::PLSTATUS);
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



