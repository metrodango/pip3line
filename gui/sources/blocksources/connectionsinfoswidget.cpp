#include "connectionsinfoswidget.h"
#include "ui_connectionsinfoswidget.h"
#include "shared/ssloptionswidget.h"
#include <QAbstractItemModel>
#include "blockssource.h"
#include "ipblockssources.h"
#include <QTableView>
#include <QDebug>

ConnectionsInfosWidget::ConnectionsInfosWidget(BlocksSource *listener , QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionsInfosWidget),
    listener(listener)
{
    ui->setupUi(this);
    enableSSLViews = false;
    peerCertsModel = nullptr;
    sslCipherLabel = nullptr;
    certTable = nullptr;

    if (listener->metaObject()->superClass()->className() == QString("IPBlocksSources")) {
        IPBlocksSources * ipbs = static_cast<IPBlocksSources *>(listener);
        if (ipbs->isTLSEnable()) {
            certTable = new(std::nothrow) QTableView();
            if (certTable == nullptr) {
                qFatal("Cannot allocate QTableView");
            }

            certTable->verticalHeader()->hide();

            peerCertsModel = new(std::nothrow) CertificatesModel();
            if (peerCertsModel == nullptr) {
                qFatal("Cannot allocate CertificatesModel");
            }

            QAbstractItemModel * omodel = certTable->model();
            certTable->setModel(peerCertsModel);
            delete omodel;
            certTable->resizeColumnsToContents();

            sslCipherLabel = new(std::nothrow) QLabel();
            if (sslCipherLabel == nullptr) {
                qFatal("Cannot allocate QLabel");
            }

            QVBoxLayout * layout = static_cast<QVBoxLayout *> (ui->mainLayout);

            if (layout != nullptr) {
                layout->addWidget(sslCipherLabel);
                layout->addWidget(certTable);
            }

            enableSSLViews = true;
        }
    }
    onConnectionUpdated();

    connect(listener, &BlocksSource::updated, this, &ConnectionsInfosWidget::onConnectionUpdated);
    connect(listener, &BlocksSource::connectionClosed, this, &ConnectionsInfosWidget::deleteLater);
}

ConnectionsInfosWidget::~ConnectionsInfosWidget()
{
    delete ui;
}

void ConnectionsInfosWidget::onConnectionUpdated()
{
    ui->ConnectionsInfoslabel->setText(listener->getDescription());
    if (enableSSLViews) {
        IPBlocksSources * ipbs = static_cast<IPBlocksSources *>(listener);
        peerCertsModel->clearCertsList();
        peerCertsModel->addCertList(ipbs->getPeerCerts());
        certTable->resizeColumnsToContents();
        sslCipherLabel->setText(ipbs->getCurrentCipher());
    }
}
