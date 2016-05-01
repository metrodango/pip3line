#include "ssloptionswidget.h"
#include "ui_ssloptionswidget.h"
#include <QSslSocket>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include "shared/guiconst.h"

const QStringList CertificatesModel::headersList = QStringList()
                            << "CN"
                            << "Expiry"
#if QT_VERSION >= 0x050400
                            << "Issuer"
                            << "CA"
                            << "selfsigned";
#else
                            << "Issuer";
#endif

#if QT_VERSION >= 0x050000

#include <QSslCertificateExtension>

#endif
#include <QSslKey>

const QString CertificatesModel::YES_STR = "yes";
const QString CertificatesModel::NO_STR = "no";

CertificatesModel::CertificatesModel(QObject *parent) :
    QAbstractTableModel(parent) {

}

CertificatesModel::~CertificatesModel()
{
    certList.clear();
}

int CertificatesModel::columnCount(const QModelIndex &) const
{
    return headersList.size();
}

int CertificatesModel::rowCount(const QModelIndex &) const
{
    return certList.size();
}

QVariant CertificatesModel::data(const QModelIndex &index, int role) const
{
    int column = index.column();
    int certIndex = index.row();
    switch (role)
    {
        case Qt::DisplayRole:
        {
            switch (column) {
                case 0: // CN
                    return QVariant(certList.at(certIndex).subjectInfo(QSslCertificate::CommonName));
                case 1: // Expiry
                    return QVariant(certList.at(certIndex).expiryDate().toString());
                case 2: // Issuer
                    return QVariant(certList.at(certIndex).issuerInfo(QSslCertificate::CommonName));
#if QT_VERSION >= 0x050400
                case 3: // is CA?
                {
                    QList<QSslCertificateExtension> extList = certList.at(certIndex).extensions();
                    for (int i = 0; i < extList.size(); i++) {
                        QSslCertificateExtension ext = extList.at(i);
                        if (ext.oid() == QString("2.5.29.19")) { // basicContraints
                             QVariantMap vmap = ext.value().toMap();
                             if (vmap.value("ca").toBool()) {
                                 return YES_STR;
                             } else
                                 return NO_STR;
                        }
                    }
                }
                    break;
                case 4: // is selfsigned ?
                    return certList.at(certIndex).isSelfSigned() ? YES_STR : NO_STR;
#endif
                default:
                    qCritical() << "unmanaged field" << index.column();
            }
        }
            break;
    }

    return QVariant();
}

QVariant CertificatesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        return headersList.at(section);
    } else {
        return QString("%1").arg(section);
    }
}
QList<QSslCertificate> CertificatesModel::getCertList() const
{
    return certList;
}

void CertificatesModel::addCertList(const QList<QSslCertificate> &value)
{
    beginResetModel();
    certList.append(value);
    endResetModel();
}

void CertificatesModel::clearCertsList()
{
    beginResetModel();
    certList.clear();
    endResetModel();
}

bool cipherLessThan(const QSslCipher &s1, const QSslCipher &s2)
{
    if (s1.protocolString() == s2.protocolString())
        return s1.name() < s2.name();
    else
        return s1.protocolString() > s2.protocolString();
}

const QStringList SSLCipherModel::headerList = QStringList() << QObject::tr("Use")
                                                             << QObject::tr("Name");

const int SSLCipherModel::CHECKBOX_COLUMN = 0;

SSLCipherModel::SSLCipherModel(const QList<QSslCipher> &cipherlist, QObject *parent) :
    QAbstractTableModel(parent)
{
    cipherList = cipherlist;
    qStableSort(cipherList.begin(),cipherList.end(), cipherLessThan);
}

SSLCipherModel::~SSLCipherModel()
{

}

int SSLCipherModel::columnCount(const QModelIndex &) const
{
    return headerList.size();
}

int SSLCipherModel::rowCount(const QModelIndex &) const
{
    return cipherList.size();
}

QVariant SSLCipherModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();


    int i = index.row();
    int col = index.column();
    switch (role)
    {
        case Qt::DisplayRole:
        {
                if (col == CHECKBOX_COLUMN) {
                    return QVariant();
                } else {
                    QSslCipher ci = cipherList.at(i);
                    switch (col) {
                        case 1:
                            return ci.name();
                    }
                }
        }
            break;
        case Qt::CheckStateRole:
            if (col == CHECKBOX_COLUMN)
                return (enabledCipherList.contains(cipherList.at(i)) ? Qt::Checked : Qt::Unchecked);
            break;
    }
    return QVariant();
}

bool SSLCipherModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole) {

        if (value.toBool()) {
            enabledCipherList.append(cipherList.at(index.row()));
        } else {
            enabledCipherList.removeAll(cipherList.at(index.row()));
        }
        emit ciphersListUpdated(enabledCipherList);
        return true;
    }

    return false;
}

QVariant SSLCipherModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        return headerList.at(section);
    } else {
        return QString("%1").arg(cipherList.at(section).protocolString());
    }
}

Qt::ItemFlags SSLCipherModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == CHECKBOX_COLUMN) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    return QAbstractItemModel::flags(index);
}

QList<QSslCipher> SSLCipherModel::getEnabledCipherList() const
{
    return cipherList;
}

void SSLCipherModel::setCipherList(const QList<QSslCipher> &value)
{
    beginResetModel();
    cipherList = value;
    endResetModel();
}

void SSLCipherModel::setEnabledCipherList(const QList<QSslCipher> &value)
{
    beginResetModel();
    enabledCipherList = value;
    endResetModel();
    emit ciphersListUpdated(enabledCipherList);
}

void SSLCipherModel::selectAllCiphers()
{
    beginResetModel();
    enabledCipherList = cipherList;
    endResetModel();
    emit ciphersListUpdated(enabledCipherList);
}

void SSLCipherModel::SelectNone()
{
    beginResetModel();
    enabledCipherList.clear();
    endResetModel();
    emit ciphersListUpdated(enabledCipherList);
}

#if QT_VERSION >= 0x050400
const QList<QSsl::KeyAlgorithm> SSLOptionsWidget::keyAlgos = QList<QSsl::KeyAlgorithm>() << QSsl::Rsa << QSsl::Dsa << QSsl::Ec;
#else
const QList<QSsl::KeyAlgorithm> SSLOptionsWidget::keyAlgos = QList<QSsl::KeyAlgorithm>() << QSsl::Rsa << QSsl::Dsa;
#endif
const QString SSLOptionsWidget::KEY_LOADED = tr("Private Key loaded");
const QString SSLOptionsWidget::KEY_NOT_LOADED = tr("Private Key Not loaded");
const QList<QSsl::EncodingFormat> SSLOptionsWidget::encodingFormats = QList<QSsl::EncodingFormat>() << QSsl::Pem << QSsl::Der;
const QStringList SSLOptionsWidget::SSL_VERIFICATION_MODES = QStringList() << tr("No verification, no asking for certificate")
                                                                           << tr("Ask for certificate, but skip validation")
                                                                           << tr("Ask for certificate, and validate it");

SSLOptionsWidget::SSLOptionsWidget(QSslConfiguration defaultconf, bool sniEnabled, QString SNIValue, bool usingSystemsCAs, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SSLOptionsWidget)
{
    sslConfiguration = defaultconf;
    ui->setupUi(this);

    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(testCheckCurrentPage(int)));
    ui->stackedWidget->setCurrentWidget(ui->certConfBlankPage); // white page by default

    clientCertLoaded = setLocalCert(sslConfiguration.localCertificate());
    clientPrivateKeyLoaded = setLocalPrivateKey(sslConfiguration.privateKey()) ;

    QList<QSslCipher> clist = QSslSocket::supportedCiphers();
    cipherModel = new(std::nothrow) SSLCipherModel(clist);
    if (cipherModel == nullptr) {
        qFatal("Cannot allocate SSLCipherModel");
    }

    cipherModel->setEnabledCipherList(sslConfiguration.ciphers());

    QAbstractItemModel * omodel = ui->ciphersTableView->model();
    ui->ciphersTableView->setModel(cipherModel);
    delete omodel;

    caCertModel = new(std::nothrow) CertificatesModel();
    if (caCertModel == nullptr) {
        qFatal("Cannot allocate CertificatesModel");
    }

    omodel = ui->certificatesCAtableView->model();
    ui->certificatesCAtableView->setModel(caCertModel);
    delete omodel;
    ui->certificatesCAtableView->verticalHeader()->hide();

    ui->sslVerifModeComboBox->addItems(SSL_VERIFICATION_MODES);
    QSslSocket::PeerVerifyMode verifmode =  sslConfiguration.peerVerifyMode();
    if (verifmode == QSslSocket::VerifyNone)
        ui->sslVerifModeComboBox->setCurrentIndex(0);
    else if (verifmode == QSslSocket::QueryPeer)
        ui->sslVerifModeComboBox->setCurrentIndex(1);
    else if (verifmode == QSslSocket::VerifyPeer)
        ui->sslVerifModeComboBox->setCurrentIndex(2);
    else
        qWarning() << tr("[SSLOptionsWidget::SSLOptionsWidget] Unmanaged SSL peer verifcation mode: %1").arg(verifmode);

    ui->sniLineEdit->setText(SNIValue);

#if QT_VERSION >= 0x050000
    ui->ciphersTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ciphersTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
#else
    ui->ciphersTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->ciphersTableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
#endif
    ui->ciphersTableView->verticalHeader()->setDefaultSectionSize(20);
    ui->ciphersTableView->resizeColumnsToContents();

    ui->sniCheckBox->setChecked(sniEnabled);
    ui->systemCACheckBox->setChecked(usingSystemsCAs);

    ui->compressionCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableCompression));
    ui->emptyFragmentsCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableEmptyFragments));
    ui->sessionTicketCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionTickets));
    ui->serverNameIndicationCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableServerNameIndication));
    ui->legacyRenegociationCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableLegacyRenegotiation));
    connect(ui->compressionCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableCompression(bool)));
    connect(ui->emptyFragmentsCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableEmptyFragments(bool)));
    connect(ui->sessionTicketCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableSessionTickets(bool)));
    connect(ui->serverNameIndicationCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableServerNameIndication(bool)));
    connect(ui->legacyRenegociationCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableLegacyRenegotiation(bool)));
#if QT_VERSION >= 0x050000
    ui->sessionCharingCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionSharing));
    ui->sessionPersistenceCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionPersistence));
    connect(ui->sessionCharingCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableSessionSharing(bool)));
    connect(ui->sessionPersistenceCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableSessionPersistence(bool)));
#else
    ui->sessionCharingCheckBox->setVisible(false);
    ui->sessionPersistenceCheckBox->setVisible(false);
#endif

    connect(cipherModel,SIGNAL(ciphersListUpdated(QList<QSslCipher>)), this, SIGNAL(sslCiphersUpdated(QList<QSslCipher>)));
    connect(ui->resetCiphersPushButton, SIGNAL(clicked(bool)), this, SLOT(onResetCiphers()));
    connect(ui->allCiphersPushButton, SIGNAL(clicked(bool)), cipherModel, SLOT(selectAllCiphers()));
    connect(ui->clearAllCiphersPushButton, SIGNAL(clicked(bool)), cipherModel, SLOT(SelectNone()));
    connect(ui->loadClientCertPushButton, SIGNAL(clicked(bool)), SLOT(onClientCertLoad()));
    connect(ui->systemCACheckBox, SIGNAL(toggled(bool)), this, SIGNAL(useOfSystemCAUpdated(bool)));

    connect(ui->sniCheckBox, SIGNAL(toggled(bool)), this, SLOT(onSniEnabled(bool)));
    connect(ui->sniLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onSNIUpdated(QString)));
    connect(ui->sslVerifModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onSSLVerifModeChanged(int)));

}

SSLOptionsWidget::~SSLOptionsWidget()
{
    delete ui;
}

void SSLOptionsWidget::setSNIValue(QString value)
{
    ui->sniLineEdit->setText(value);
}

void SSLOptionsWidget::onResetCiphers()
{
    QList<QSslCipher> clist = QSslSocket::defaultCiphers();
    cipherModel->setEnabledCipherList(clist);
}

void SSLOptionsWidget::onClientCertLoad()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"),QDir::home().absolutePath(), tr("PEM (*.pem)"));

    if (!fileName.isEmpty()) {
        QSslKey clientKey;

        QByteArray passphrase("testing");
        QFile certFile(fileName);
        if (certFile.open(QIODevice::ReadOnly)) {
            bool gotCert = false;
            bool gotKey = false;
            for (int i = 0; i < encodingFormats.size(); ++i) {
                QList<QSslCertificate> list = QSslCertificate::fromDevice(&certFile, encodingFormats.at(i));
                if (!list.isEmpty()) {
                    if (list.size() > 1)
                        QMessageBox::warning(this, tr("Multiple Certs"), tr("Multiple certificates loaded, considering only the first one"),QMessageBox::Ok);
                    gotCert = setLocalCert(list.at(0));
                    if (gotCert)
                        break;
                }
             }

            for (int i = 0; i < encodingFormats.size(); ++i) {
                for (int j = 0; j < keyAlgos.size(); ++j) {
                    certFile.seek(0); // reseting, just in case
                    clientKey = QSslKey(&certFile,keyAlgos.at(j),encodingFormats.at(i),QSsl::PrivateKey, passphrase);
                    gotKey = setLocalPrivateKey(clientKey);
                    if (gotKey)
                        break;
                }
                if (gotKey)
                    break;
            }

            if (gotCert && gotKey) {
                QMessageBox::information(this, tr("Certificate and Private Key loaded"), tr("One Certificate and one private key have been loaded, all good."),QMessageBox::Ok);
                clientPrivateKeyLoaded = true;
                clientCertLoaded = true;
            } else if (gotCert) {
                QMessageBox::information(this, tr("Certificate loaded"), tr("Only a certificate has been loaded (no private key)"),QMessageBox::Ok);
                clientCertLoaded = true;
            } else if (gotKey) {
                QMessageBox::information(this, tr("Private Key loaded"), tr("Only a private key has been loaded (no certificate)"),QMessageBox::Ok);
                clientPrivateKeyLoaded = true;
            } else { // failed to load anything
                QMessageBox::critical(this, tr("Fail"), tr("Failed to read a key or a certificate"),QMessageBox::Ok);
            }

        } else {
            QMessageBox::critical(this, tr("Failed to open file"), tr("Cannot open the client cert file"),QMessageBox::Ok);
        }
    }
}

void SSLOptionsWidget::onSSLVerifModeChanged(int index)
{
    if (index == 0)
        emit peerVerificationModeChanged(QSslSocket::VerifyNone);
    else if (index == 1)
        emit peerVerificationModeChanged(QSslSocket::QueryPeer);
    else if (index == 2)
        emit peerVerificationModeChanged(QSslSocket::VerifyPeer);
    else
        qCritical() << tr("[SSLOptionsWidget::onSSLVerifModeChanged] Unknown selection value T_T");
}

void SSLOptionsWidget::onSniEnabled(bool enabled)
{
    ui->sniLineEdit->setEnabled(enabled);
    emit sniEnabled(enabled);
}

void SSLOptionsWidget::onSNIUpdated(QString value)
{
    if (QUrl::toAce(value).isEmpty()) {
        ui->sniLineEdit->setStyleSheet("QLineEdit { color : red; }");
        ui->sniLineEdit->setToolTip(tr("Invalid host name, it will be ignored for the TLS connection"));
    } else {
        ui->sniLineEdit->setStyleSheet("");
    }

    emit peerVerificationNameChanged(value);
}

void SSLOptionsWidget::testCheckCurrentPage(int page)
{
    qDebug() << tr("Page changed") << page;
}

QString SSLOptionsWidget::concat(const QStringList &list)
{
    QString temp;
    for (int i = 0; i < list.size(); i++) {
        temp.append(list.at(i)).append("\n");
    }
    temp.chop(1);
    return temp;
}

QString SSLOptionsWidget::prettyHex(const QByteArray &data)
{
    QString temp;

    QByteArray bdata = data.toHex();

    for (int i = 0 ; i < data.size(); i++) {
        temp.append(QString::fromUtf8(bdata.mid(i * 2,2)))
                .append(":");
    }
    temp.chop(1);
    return temp;
}

bool SSLOptionsWidget::setLocalCert(const QSslCertificate &cert)
{
    bool ret = false;
    if (!cert.isNull()) {
#if QT_VERSION >= 0x050000
        ui->subjectDataLabel->setText(concat(cert.subjectInfo(QSslCertificate::CommonName)));
        ui->IssuedByDataLabel->setText(concat(cert.issuerInfo(QSslCertificate::CommonName)));
#else
        ui->subjectDataLabel->setText(clientCert.subjectInfo(QSslCertificate::CommonName));
        ui->IssuedByDataLabel->setText(clientCert.issuerInfo(QSslCertificate::CommonName));
#endif
        ui->validityFromDataLabel->setText(cert.effectiveDate().toString());
        ui->validityUntilDataLabel->setText(cert.expiryDate().toString());
        ui->md5DataLabel->setText(prettyHex(cert.digest(QCryptographicHash::Md5)));
        ui->sha1DataLabel->setText(prettyHex(cert.digest(QCryptographicHash::Sha1)));
        ui->sha256DataLabel->setText(prettyHex(cert.digest(QCryptographicHash::Sha256)));
        ui->stackedWidget->setCurrentWidget(ui->certConfPage);
        emit sslLocalCertUpdated(cert);
        ret = true;
        qDebug()<< "got cert";
    }

    return ret;
}

bool SSLOptionsWidget::setLocalPrivateKey(const QSslKey &key)
{
    bool ret = false;
    if (!key.isNull()) {
        ui->privateKeyLabel->setStyleSheet("QLabel { color : green; }");
        ui->privateKeyLabel->setText(KEY_LOADED);
        qDebug()<< "got private key";
        emit sslLocalKeyUpdated(key);
        ret = true;
    } else {
        ui->privateKeyLabel->setStyleSheet("QLabel { color : red; }");
        ui->privateKeyLabel->setText(KEY_NOT_LOADED);
    }

    return ret;
}


