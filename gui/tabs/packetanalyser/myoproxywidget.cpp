#include "myoproxywidget.h"
#include "myoproxy.h"
#include "ui_myoproxywidget.h"

MYOProxyWidget::MYOProxyWidget(MYOProxy *proxy, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MYOProxyWidget),
    proxy(proxy)
{
    ui->setupUi(this);

    QStringList servers;
    servers << tr("Choose");
    servers.append(MYOProxy::SERVERS_LIST);

    QStringList clients;
    clients << tr("Choose");
    clients.append(MYOProxy::CLIENTS_LIST);

    ui->serverComboBox->addItems(servers);
    ui->serverComboBox->setCurrentIndex(proxy->getServerType());

    ui->clientComboBox->addItems(clients);
    ui->clientComboBox->setCurrentIndex(proxy->getClientType());

    connect(ui->serverComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onServerChanged(int)));
    connect(ui->clientComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onClientChanged(int)));
}

MYOProxyWidget::~MYOProxyWidget()
{
    delete ui;
}

void MYOProxyWidget::setServerWidget(QWidget *gui)
{
    ui->serverTab->layout()->addWidget(gui);
}

void MYOProxyWidget::setClientWidget(QWidget *gui)
{
    ui->clientTab->layout()->addWidget(gui);
}

void MYOProxyWidget::onServerChanged(int index)
{

    if (index > 0)
        proxy->setServer(static_cast<MYOProxy::SERVERS>(index));

    ui->tabWidget->setCurrentIndex(0);
}

void MYOProxyWidget::onClientChanged(int index)
{
    if (index > 0)
        proxy->setClient(static_cast<MYOProxy::CLIENTS>(index));

    ui->tabWidget->setCurrentIndex(1);
}
