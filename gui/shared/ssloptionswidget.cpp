#include "ssloptionswidget.h"
#include "ui_ssloptionswidget.h"
#include <QSslSocket>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include "shared/guiconst.h"
#include <QUrl>

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

#include <QSslCertificateExtension>
#include <QSslKey>
#include <QCollator>

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
    emit certListUpdated(certList);
}

void CertificatesModel::clearCertsList()
{
    beginResetModel();
    certList.clear();
    endResetModel();
    emit certListUpdated(certList);
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
    QAbstractTableModel(parent),
    cipherList(cipherlist)
{
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
    return enabledCipherList;
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

void SSLCipherModel::enableProtocol(QSsl::SslProtocol protocol, bool enable)
{
    beginResetModel();
    for (int i = 0; i < cipherList.size(); i++) {
        QSslCipher cipher = cipherList.at(i);
        if (cipher.protocol() == protocol) {
            if (enable) {
                if (!enabledCipherList.contains(cipher)) {
                    enabledCipherList.append(cipher);
                }
            } else {
                if (enabledCipherList.contains(cipher)) {
                    enabledCipherList.removeAll(cipher);
                }
            }
        }
    }
    endResetModel();
    emit ciphersListUpdated(enabledCipherList);
}

bool SSLCipherModel::hasProtocol(QSsl::SslProtocol protocol)
{
    for (int i = 0; i < cipherList.size(); i++) {
        if (cipherList.at(i).protocol() == protocol)
            return true;
    }

    return false;
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

bool curvesLessThan(const QSslEllipticCurve &c1, const QSslEllipticCurve &c2)
{
    QCollator comp;
    comp.setNumericMode(true);
    return comp.compare(c1.shortName(), c2.shortName()) < 0;
}

const QStringList SSLCurvesModel::headerList = QStringList() << QObject::tr("Use")
                                                             << QObject::tr("Name");

const int SSLCurvesModel::CHECKBOX_COLUMN = 0;

SSLCurvesModel::SSLCurvesModel(const QVector<QSslEllipticCurve> &curvesList, QObject *parent) :
    QAbstractTableModel(parent),
    fullCurvesList(curvesList)
{
    qStableSort(fullCurvesList.begin(),fullCurvesList.end(), curvesLessThan);
}

SSLCurvesModel::~SSLCurvesModel()
{

}

int SSLCurvesModel::columnCount(const QModelIndex &) const
{
    return headerList.size();
}

int SSLCurvesModel::rowCount(const QModelIndex &) const
{
    return fullCurvesList.size();
}

QVariant SSLCurvesModel::data(const QModelIndex &index, int role) const
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
                    QSslEllipticCurve cu = fullCurvesList.at(i);
                    if (col == 1)
                            return cu.shortName();
                }
        }
            break;
        case Qt::CheckStateRole:
            if (col == CHECKBOX_COLUMN) {
                QSslEllipticCurve cu = fullCurvesList.at(i);
                QVariant ret = (selectedCurvesList.contains(cu) ? Qt::Checked : Qt::Unchecked);
                return ret;
            }
            break;
    }
    return QVariant();
}

QVariant SSLCurvesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

    if (orientation == Qt::Horizontal) {
        return headerList.at(section);
    } else {
        return QString("%1").arg(section);
    }
}

bool SSLCurvesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole) {
        if (value.toBool()) {
            selectedCurvesList.append(fullCurvesList.at(index.row()));
        } else {
            selectedCurvesList.removeAll(fullCurvesList.at(index.row()));
        }
        emit selectedCurvesListUpdated(selectedCurvesList);
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

Qt::ItemFlags SSLCurvesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    if (index.column() == CHECKBOX_COLUMN) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    return QAbstractItemModel::flags(index);
}

QVector<QSslEllipticCurve> SSLCurvesModel::getEnabledCurvesList() const
{
    return selectedCurvesList;
}

void SSLCurvesModel::setCurvesList(const QVector<QSslEllipticCurve> &value)
{
    beginResetModel();
    fullCurvesList = value;
    endResetModel();
}

void SSLCurvesModel::setEnabledCurvesList(const QVector<QSslEllipticCurve> &value)
{
    beginResetModel();
    selectedCurvesList = value;
    endResetModel();
    emit selectedCurvesListUpdated(selectedCurvesList);
}

void SSLCurvesModel::selectAllCurves()
{
    beginResetModel();
    selectedCurvesList = fullCurvesList;
    endResetModel();
    emit selectedCurvesListUpdated(selectedCurvesList);
}

void SSLCurvesModel::SelectNone()
{
    beginResetModel();
    selectedCurvesList.clear();
    endResetModel();
    emit selectedCurvesListUpdated(selectedCurvesList);
}


#if QT_VERSION >= 0x050400
const QList<QSsl::KeyAlgorithm> SSLOptionsWidget::keyAlgos = QList<QSsl::KeyAlgorithm>() << QSsl::Rsa << QSsl::Dsa << QSsl::Ec;
#else
const QList<QSsl::KeyAlgorithm> SSLOptionsWidget::keyAlgos = QList<QSsl::KeyAlgorithm>() << QSsl::Rsa << QSsl::Dsa;
#endif
const QString SSLOptionsWidget::KEY_LOADED = QObject::tr("Private Key loaded");
const QString SSLOptionsWidget::KEY_NOT_LOADED = QObject::tr("Private Key Not loaded");
const QString SSLOptionsWidget::UNAVAILABLE_STR = QString("Protocol not available in the current version of OpenSSL");
const QList<QSsl::EncodingFormat> SSLOptionsWidget::encodingFormats = QList<QSsl::EncodingFormat>() << QSsl::Pem << QSsl::Der;
const QStringList SSLOptionsWidget::SSL_VERIFICATION_MODES = QStringList() << QObject::tr("No verification, no asking for certificate")
                                                                           << QObject::tr("Ask for certificate, but skip validation")
                                                                           << QObject::tr("Ask for certificate, and validate it");

SSLOptionsWidget::SSLOptionsWidget(QSslConfiguration defaultconf,
                                   bool sniEnabled,
                                   QString SNIValue,
                                   bool usingSystemsCAs,
                                   const QList<QSslCertificate> &otherCAs,
                                   QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SSLOptionsWidget)
{
    sslConfiguration = defaultconf;
    ui->setupUi(this);

    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(testCheckCurrentPage(int)));
    ui->stackedWidget->setCurrentWidget(ui->certConfBlankPage); // white page by default

    allowedNextProtocols = defaultconf.allowedNextProtocols();
    ui->http11CheckBox->setChecked(allowedNextProtocols.contains(QByteArray(QSslConfiguration::NextProtocolHttp1_1)));
    ui->sdpyCheckBox->setChecked(allowedNextProtocols.contains(QByteArray(QSslConfiguration::NextProtocolSpdy3_0)));

    clientCertLoaded = setLocalCert(sslConfiguration.localCertificate());
    clientPrivateKeyLoaded = setLocalPrivateKey(sslConfiguration.privateKey()) ;

    QList<QSslCipher> clist = QSslSocket::supportedCiphers();
    cipherModel = new(std::nothrow) SSLCipherModel(clist);
    if (cipherModel == nullptr) {
        qFatal("Cannot allocate SSLCipherModel");
    }

    cipherModel->setEnabledCipherList(sslConfiguration.ciphers());

    if (!cipherModel->hasProtocol(QSsl::SslV2)) {
        ui->sslv2checkBox->setEnabled(false);
        ui->sslv2checkBox->setToolTip(UNAVAILABLE_STR);
    }

    if (!cipherModel->hasProtocol(QSsl::SslV3)) {
        ui->sslv3checkBox->setEnabled(false);
        ui->sslv3checkBox->setToolTip(UNAVAILABLE_STR);
    }

    if (!cipherModel->hasProtocol(QSsl::TlsV1_0)) {
        ui->tlsv10checkBox->setEnabled(false);
        ui->tlsv10checkBox->setToolTip(UNAVAILABLE_STR);
    }

    if (!cipherModel->hasProtocol(QSsl::TlsV1_1)) {
        ui->tlsv11checkBox->setEnabled(false);
        ui->tlsv11checkBox->setToolTip(UNAVAILABLE_STR);
    }

    if (!cipherModel->hasProtocol(QSsl::TlsV1_2)) {
        ui->tlsv12checkBox->setEnabled(false);
        ui->tlsv12checkBox->setToolTip(UNAVAILABLE_STR);
    }

    QAbstractItemModel * omodel = ui->ciphersTableView->model();
    ui->ciphersTableView->setModel(cipherModel);
    delete omodel;

    ui->ciphersTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ciphersTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ciphersTableView->verticalHeader()->setDefaultSectionSize(20);
    ui->ciphersTableView->resizeColumnsToContents();
    ui->ciphersTableView->setSelectionMode(QAbstractItemView::NoSelection);

    QVector<QSslEllipticCurve> ecList = QSslConfiguration::supportedEllipticCurves();
    curvesModel = new(std::nothrow) SSLCurvesModel(ecList);
    if (curvesModel == nullptr) {
        qFatal("Cannot allocate SSLCurvesModel");
    }

    curvesModel->setEnabledCurvesList(sslConfiguration.ellipticCurves());

    omodel = ui->ecTableView->model();
    ui->ecTableView->setModel(curvesModel);
    delete omodel;
  //  ui->ecTableView->verticalHeader()->hide();

    ui->ecTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ecTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->ecTableView->verticalHeader()->setDefaultSectionSize(20);
    ui->ecTableView->resizeColumnsToContents();
    ui->ecTableView->setSelectionMode(QAbstractItemView::NoSelection);

    caCertModel = new(std::nothrow) CertificatesModel();
    if (caCertModel == nullptr) {
        qFatal("Cannot allocate CertificatesModel");
    }

    caCertModel->addCertList(otherCAs);

    omodel = ui->certificatesCAtableView->model();
    ui->certificatesCAtableView->setModel(caCertModel);
    delete omodel;
    ui->certificatesCAtableView->verticalHeader()->hide();
    ui->certificatesCAtableView->resizeColumnsToContents();
    ui->certificatesCAtableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->certificatesCAtableView->setSelectionMode(QAbstractItemView::SingleSelection);

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
    ui->sniLineEdit->setEnabled(sniEnabled);
    ui->sniCheckBox->setChecked(sniEnabled);

    ui->systemCACheckBox->setChecked(usingSystemsCAs);

    ui->compressionCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableCompression));
    ui->emptyFragmentsCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableEmptyFragments));
    ui->sessionTicketCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionTickets));

    ui->legacyRenegociationCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableLegacyRenegotiation));
    connect(ui->compressionCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableCompression(bool)));
    connect(ui->emptyFragmentsCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableEmptyFragments(bool)));
    connect(ui->sessionTicketCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableSessionTickets(bool)));
    connect(ui->legacyRenegociationCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableLegacyRenegotiation(bool)));

    ui->sessionCharingCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionSharing));
    ui->sessionPersistenceCheckBox->setChecked(sslConfiguration.testSslOption(QSsl::SslOptionDisableSessionPersistence));
    connect(ui->sessionCharingCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableSessionSharing(bool)));
    connect(ui->sessionPersistenceCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(disableSessionPersistence(bool)));

    connect(cipherModel,SIGNAL(ciphersListUpdated(QList<QSslCipher>)), this, SIGNAL(sslCiphersUpdated(QList<QSslCipher>)));
    connect(ui->resetCiphersPushButton, SIGNAL(clicked(bool)), this, SLOT(onResetCiphers()));
    connect(ui->allCiphersPushButton, SIGNAL(clicked(bool)), this, SLOT(onSelectAllCiphers()));
    connect(ui->clearAllCiphersPushButton, SIGNAL(clicked(bool)), this, SLOT(onSelectNoneCipher()));
    connect(ui->loadClientCertPushButton, SIGNAL(clicked(bool)), SLOT(onClientCertLoad()));
    connect(ui->systemCACheckBox, SIGNAL(toggled(bool)), this, SIGNAL(useOfSystemCAUpdated(bool)));

    connect(ui->noneECPushButton, SIGNAL(clicked(bool)), this, SLOT(onSelectNoneCurves()));
    connect(curvesModel, SIGNAL(selectedCurvesListUpdated(QVector<QSslEllipticCurve>)), this , SIGNAL(selectedCurvesListUpdated(QVector<QSslEllipticCurve>)));

    connect(caCertModel, SIGNAL(certListUpdated(QList<QSslCertificate>)), this, SIGNAL(caListUpdated(QList<QSslCertificate>)));
    connect(ui->clearAllCAPushButton, SIGNAL(clicked(bool)), caCertModel, SLOT(clearCertsList()));
    connect(ui->loadCAPushButton, SIGNAL(clicked(bool)), this, SLOT(onCALoad()));

    connect(ui->sniCheckBox, SIGNAL(toggled(bool)), this, SLOT(onSniEnabled(bool)));
    connect(ui->sniLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onSNIUpdated(QString)));
    connect(ui->sslVerifModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onSSLVerifModeChanged(int)));

    connect(ui->sslv2checkBox, SIGNAL(toggled(bool)), this, SLOT(onSSLv2Toggled(bool)));
    connect(ui->sslv3checkBox, SIGNAL(toggled(bool)), this, SLOT(onSSLv3Toggled(bool)));
    connect(ui->tlsv10checkBox, SIGNAL(toggled(bool)), this, SLOT(onTLsv10Toggled(bool)));
    connect(ui->tlsv11checkBox, SIGNAL(toggled(bool)), this, SLOT(onTLsv11Toggled(bool)));
    connect(ui->tlsv12checkBox, SIGNAL(toggled(bool)), this, SLOT(onTLsv12Toggled(bool)));

    connect(ui->http11CheckBox, SIGNAL(toggled(bool)), this, SLOT(onHTTP11NextProtocolToggled(bool)));
    connect(ui->sdpyCheckBox, SIGNAL(toggled(bool)), this, SLOT(onSDPYNextProtocolToggled(bool)));
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
    if (ui->sslv2checkBox->isEnabled()) {
        ui->sslv2checkBox->blockSignals(true);
        ui->sslv2checkBox->setChecked(false);
        ui->sslv2checkBox->blockSignals(false);
    }

    if (ui->sslv3checkBox->isEnabled()) {
        ui->sslv3checkBox->blockSignals(true);
        ui->sslv3checkBox->setChecked(false);
        ui->sslv3checkBox->blockSignals(false);
    }

    if (ui->tlsv10checkBox->isEnabled()) {
        ui->tlsv10checkBox->blockSignals(true);
        ui->tlsv10checkBox->setChecked(false);
        ui->tlsv10checkBox->blockSignals(false);
    }

    if (ui->tlsv11checkBox->isEnabled()) {
        ui->tlsv11checkBox->blockSignals(true);
        ui->tlsv11checkBox->setChecked(false);
        ui->tlsv11checkBox->blockSignals(false);
    }

    if (ui->tlsv12checkBox->isEnabled()) {
        ui->tlsv12checkBox->blockSignals(true);
        ui->tlsv12checkBox->setChecked(false);
        ui->tlsv12checkBox->blockSignals(false);
    }
}

void SSLOptionsWidget::onClientCertLoad()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this,tr("Choose file to load from"),GuiConst::GLOBAL_LAST_PATH, tr("PEM (*.pem)"));

    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
        QSslKey clientKey;

        QByteArray passphrase = ui->keyPassLineEdit->text().toUtf8();
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
                certFile.seek(0); // need to come back at the beginning
             }

            for (int i = 0; i < encodingFormats.size(); ++i) {
                for (int j = 0; j < keyAlgos.size(); ++j) {
                    certFile.seek(0); // reseting, just in case
                    clientKey = QSslKey(&certFile,keyAlgos.at(j),encodingFormats.at(i),QSsl::PrivateKey, passphrase);
                    if (!clientKey.isNull())
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

void SSLOptionsWidget::onCALoad()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this,tr("Choose certificate(s) file to load"),GuiConst::GLOBAL_LAST_PATH, tr("Certificate (*.pem *.cer *.der *.crt)"));

    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        GuiConst::GLOBAL_LAST_PATH = fi.absoluteFilePath();
        QFile certFile(fileName);
        if (certFile.open(QIODevice::ReadOnly)) {
            bool gotCert = false;
            for (int i = 0; i < encodingFormats.size(); ++i) {
                QList<QSslCertificate> list = QSslCertificate::fromDevice(&certFile, encodingFormats.at(i));
                if (!list.isEmpty()) {
                    qDebug();
                    gotCert = true;
                    caCertModel->addCertList(list);
                    break; // we successfully read the cert list, exiting loop
                }
                certFile.seek(0); // need to come back at the beginning
             }

            if (!gotCert) {
                QMessageBox::critical(this, tr("Fail"), tr("Failed to read any CA certificate"),QMessageBox::Ok);
            }

        } else {
            QMessageBox::critical(this, tr("Failed to open file"), tr("Cannot open the CA cert file"),QMessageBox::Ok);
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

void SSLOptionsWidget::onSelectAllCiphers()
{
    if (ui->sslv2checkBox->isEnabled()) {
        ui->sslv2checkBox->blockSignals(true);
        ui->sslv2checkBox->setChecked(true);
        ui->sslv2checkBox->blockSignals(false);
    }

    if (ui->sslv3checkBox->isEnabled()) {
        ui->sslv3checkBox->blockSignals(true);
        ui->sslv3checkBox->setChecked(true);
        ui->sslv3checkBox->blockSignals(false);
    }

    if (ui->tlsv10checkBox->isEnabled()) {
        ui->tlsv10checkBox->blockSignals(true);
        ui->tlsv10checkBox->setChecked(true);
        ui->tlsv10checkBox->blockSignals(false);
    }

    if (ui->tlsv11checkBox->isEnabled()) {
        ui->tlsv11checkBox->blockSignals(true);
        ui->tlsv11checkBox->setChecked(true);
        ui->tlsv11checkBox->blockSignals(false);
    }

    if (ui->tlsv12checkBox->isEnabled()) {
        ui->tlsv12checkBox->blockSignals(true);
        ui->tlsv12checkBox->setChecked(true);
        ui->tlsv12checkBox->blockSignals(false);
    }

    cipherModel->selectAllCiphers();
}

void SSLOptionsWidget::onSelectNoneCipher()
{
    if (ui->sslv2checkBox->isEnabled()) {
        ui->sslv2checkBox->blockSignals(true);
        ui->sslv2checkBox->setChecked(false);
        ui->sslv2checkBox->blockSignals(false);
    }

    if (ui->sslv3checkBox->isEnabled()) {
        ui->sslv3checkBox->blockSignals(true);
        ui->sslv3checkBox->setChecked(false);
        ui->sslv3checkBox->blockSignals(false);
    }

    if (ui->tlsv10checkBox->isEnabled()) {
        ui->tlsv10checkBox->blockSignals(true);
        ui->tlsv10checkBox->setChecked(false);
        ui->tlsv10checkBox->blockSignals(false);
    }

    if (ui->tlsv11checkBox->isEnabled()) {
        ui->tlsv11checkBox->blockSignals(true);
        ui->tlsv11checkBox->setChecked(false);
        ui->tlsv11checkBox->blockSignals(false);
    }

    if (ui->tlsv12checkBox->isEnabled()) {
        ui->tlsv12checkBox->blockSignals(true);
        ui->tlsv12checkBox->setChecked(false);
        ui->tlsv12checkBox->blockSignals(false);
    }

    cipherModel->SelectNone();
}

void SSLOptionsWidget::onSelectNoneCurves()
{
    curvesModel->SelectNone();
}

void SSLOptionsWidget::onSSLv2Toggled(bool checked)
{
    cipherModel->enableProtocol(QSsl::SslV2, checked);
}

void SSLOptionsWidget::onSSLv3Toggled(bool checked)
{
    cipherModel->enableProtocol(QSsl::SslV3, checked);
}

void SSLOptionsWidget::onTLsv10Toggled(bool checked)
{
    cipherModel->enableProtocol(QSsl::TlsV1_0, checked);
}

void SSLOptionsWidget::onTLsv11Toggled(bool checked)
{
    cipherModel->enableProtocol(QSsl::TlsV1_1, checked);
}

void SSLOptionsWidget::onTLsv12Toggled(bool checked)
{
    cipherModel->enableProtocol(QSsl::TlsV1_2, checked);
}

void SSLOptionsWidget::onHTTP11NextProtocolToggled(bool checked)
{
    if (checked) {
        if (!allowedNextProtocols.contains(QByteArray(QSslConfiguration::NextProtocolHttp1_1))) {
            allowedNextProtocols.append(QByteArray(QSslConfiguration::NextProtocolHttp1_1));
        }
    } else {
        allowedNextProtocols.removeAll(QByteArray(QSslConfiguration::NextProtocolHttp1_1));
    }

    emit allowedNextProtocolsUpdated(allowedNextProtocols);
}

void SSLOptionsWidget::onSDPYNextProtocolToggled(bool checked)
{
    if (checked) {
        if (!allowedNextProtocols.contains(QByteArray(QSslConfiguration::NextProtocolSpdy3_0))) {
            allowedNextProtocols.append(QByteArray(QSslConfiguration::NextProtocolSpdy3_0));
        }
    } else {
        allowedNextProtocols.removeAll(QByteArray(QSslConfiguration::NextProtocolSpdy3_0));
    }

    emit allowedNextProtocolsUpdated(allowedNextProtocols);
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

        ui->subjectDataLabel->setText(concat(cert.subjectInfo(QSslCertificate::CommonName)));
        ui->IssuedByDataLabel->setText(concat(cert.issuerInfo(QSslCertificate::CommonName)));

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
