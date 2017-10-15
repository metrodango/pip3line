#ifndef SSLOPTIONSWIDGET_H
#define SSLOPTIONSWIDGET_H

#include <QAbstractTableModel>
#include <QSslConfiguration>
#include <QWidget>
#include <QSslCipher>
#include <QSslEllipticCurve>

class CertificatesModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit CertificatesModel(QObject *parent = 0);
        ~CertificatesModel();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        QList<QSslCertificate> getCertList() const;
        void addCertList(const QList<QSslCertificate> &value);
    public slots:
        void clearCertsList();
    signals:
        void certListUpdated(const QList<QSslCertificate> &cacerts);

    private:
        QList<QSslCertificate> certList;
        static const QStringList headersList;
        static const QString YES_STR;
        static const QString NO_STR;
};

class SSLCipherModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit SSLCipherModel(const QList<QSslCipher> &cipherlist, QObject *parent = 0);
        ~SSLCipherModel();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role);
        Qt::ItemFlags flags(const QModelIndex &index) const;

        QList<QSslCipher> getEnabledCipherList() const;
        void setCipherList(const QList<QSslCipher> &value);
        void setEnabledCipherList(const QList<QSslCipher> &value);
        void enableProtocol(QSsl::SslProtocol protocol, bool enable);
        bool hasProtocol(QSsl::SslProtocol protocol);
    public slots:
        void selectAllCiphers();
        void SelectNone();

    signals:
        void ciphersListUpdated(const QList<QSslCipher> & ciphers);
    private:
        static const QStringList headerList;
        static const int CHECKBOX_COLUMN;
        QList<QSslCipher> cipherList;
        QList<QSslCipher> enabledCipherList;
};

class SSLCurvesModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit SSLCurvesModel(const QVector<QSslEllipticCurve> &curvesList, QObject *parent = 0);
        ~SSLCurvesModel();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role);
        Qt::ItemFlags flags(const QModelIndex &index) const;

        QVector<QSslEllipticCurve> getEnabledCurvesList() const;
        void setCurvesList(const QVector<QSslEllipticCurve> &value);
        void setEnabledCurvesList(const QVector<QSslEllipticCurve> &value);
    public slots:
        void selectAllCurves();
        void SelectNone();
    signals:
        void selectedCurvesListUpdated(const QVector<QSslEllipticCurve> & curves);
    private:
        static const QStringList headerList;
        static const int CHECKBOX_COLUMN;
        QVector<QSslEllipticCurve> fullCurvesList;
        QVector<QSslEllipticCurve> selectedCurvesList;
};

namespace Ui {
    class SSLOptionsWidget;
}

class SSLOptionsWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit SSLOptionsWidget(QSslConfiguration defaultconf,
                                  bool sniEnabled,
                                  QString SNIValue,
                                  bool usingSystemsCAs,
                                  const QList<QSslCertificate> &otherCAs,
                                  QWidget *parent = 0);
        ~SSLOptionsWidget();
    public slots:
        void setSNIValue(QString value);
    private slots:
        void onResetCiphers();
        void onClientCertLoad();
        void onCALoad();
        void onSSLVerifModeChanged(int index);
        void onSniEnabled(bool enabled);
        void onSNIUpdated(QString value);
        void testCheckCurrentPage(int page);
        void onSelectAllCiphers();
        void onSelectNoneCipher();
        void onSelectNoneCurves();
        void onSSLv2Toggled(bool checked);
        void onSSLv3Toggled(bool checked);
        void onTLsv10Toggled(bool checked);
        void onTLsv11Toggled(bool checked);
        void onTLsv12Toggled(bool checked);
        void onHTTP11NextProtocolToggled(bool checked);
        void onSDPYNextProtocolToggled(bool checked);
    signals:
        void useOfSystemCAUpdated(bool usesSystemCAs);
        void caListUpdated(const QList<QSslCertificate> & certificates);
        void sslCiphersUpdated(const QList<QSslCipher> & ciphers);
        void sslLocalCertUpdated(const QSslCertificate &cert);
        void sslLocalKeyUpdated(const QSslKey &key);
        void disableEmptyFragments(bool disable);
        void disableSessionTickets(bool disable);
        void disableCompression(bool disable);
        void disableLegacyRenegotiation(bool disable);
        void disableSessionSharing(bool disable);
        void disableSessionPersistence(bool disable);
        void peerVerificationModeChanged(QSslSocket::PeerVerifyMode);
        void peerVerificationNameChanged(QString);
        void sniEnabled(bool enabled);
        void allowedNextProtocolsUpdated(const QList<QByteArray> &value);
        void selectedCurvesListUpdated(const QVector<QSslEllipticCurve> & curves);
    private:
        static const QStringList SSL_VERIFICATION_MODES;
        QString concat(const QStringList &list);
        QString prettyHex(const QByteArray &data);
        bool setLocalCert(const QSslCertificate &cert);
        bool setLocalPrivateKey(const QSslKey &key);
        Ui::SSLOptionsWidget *ui;
        QSslConfiguration sslConfiguration;
        SSLCipherModel *cipherModel;
        CertificatesModel *caCertModel;
        SSLCurvesModel * curvesModel;
        static const QList<QSsl::KeyAlgorithm> keyAlgos;
        static const QList<QSsl::EncodingFormat> encodingFormats;
        static const QString KEY_LOADED;
        static const QString KEY_NOT_LOADED;
        static const QString UNAVAILABLE_STR;
        bool clientCertLoaded;
        bool clientPrivateKeyLoaded;
        QList<QByteArray> allowedNextProtocols;
};

#endif // SSLOPTIONSWIDGET_H
