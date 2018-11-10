#include "ipnetworkserverwidget.h"
#include "ui_ipnetworkserverwidget.h"
#include "ipblockssources.h"
#include "connectionswidget.h"
#include "shared/guiconst.h"
#include <QNetworkInterface>

IPNetworkServerWidget::IPNetworkServerWidget(IPBlocksSources *nlistener, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IPNetworkServerWidget)
{
    listener = nlistener;
    ui->setupUi(this);
    refreshIPs();

    ui->portSpinBox->setMinimum(0);
    ui->portSpinBox->setMaximum(USHRT_MAX);
    ui->portSpinBox->setValue(listener->getHostPort());
    QString addr = listener->getHostAddress().toString();
    int index = ui->bindToComboBox->findText(addr);
    if (index != -1)
        ui->bindToComboBox->setCurrentIndex(index);

    //connect(ui->portSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &IPNetworkServerWidget::onPortChanged);
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPortChanged(int)));
    //connect(ui->bindToComboBox, qOverload<const QString &>(&QComboBox::currentIndexChanged), this, &IPNetworkServerWidget::onIPChanged);
    connect(ui->bindToComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onIPChanged(QString)));
    connect(ui->refreshPushButton, &QPushButton::clicked, this, &IPNetworkServerWidget::refreshIPs);
}

IPNetworkServerWidget::~IPNetworkServerWidget()
{
    delete ui;
}

void IPNetworkServerWidget::onTlsToggled(bool enabled)
{
    // for easier use only
    if (enabled && ui->portSpinBox->value() == 80) {
        ui->portSpinBox->setValue(443);
    } else if (! enabled && ui->portSpinBox->value() == 443) {
        ui->portSpinBox->setValue(80);
    }
}

void IPNetworkServerWidget::refreshIPs()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    QList<QString> slist;

    for (int i = 0; i < list.size(); i++) {
        slist.append(list.at(i).toString());
    }
    int index = slist.indexOf("127.0.0.1");
    if (index == -1)
        index = 0;

    ui->bindToComboBox->clear();
    ui->bindToComboBox->addItems(slist);
    ui->bindToComboBox->setCurrentIndex(index);
}

void IPNetworkServerWidget::onPortChanged(int port)
{
    listener->setHostPort(static_cast<quint16>(port));
}

void IPNetworkServerWidget::onIPChanged(const QString &value)
{
    listener->setHostAddress(QHostAddress(value));
}


