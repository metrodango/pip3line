#include "ipnetworkclientwidget.h"
#include "ui_ipnetworkclientwidget.h"
#include "ipblockssources.h"
#include "connectionswidget.h"
#include <QDebug>

IPNetworkClientWidget::IPNetworkClientWidget(IPBlocksSources *nlistener, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IPNetworkClientWidget),
    listener(nlistener)
{
    resolutionTimer.setInterval(500);
    resolutionTimer.setSingleShot(true);
    connect(&resolutionTimer, SIGNAL(timeout()), SLOT(onResolutionTimerExpired()));

    ui->setupUi(this);

    ui->portSpinBox->setValue((int)listener->getHostPort());

    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPortChanged(int)));
    connect(ui->peerAddrLineEdit, SIGNAL(textChanged(QString)), SLOT(onTargetchanged()));
    connect(ui->hostResolvComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(onIPChanged(QString)));

    //need to do taht after, to get dns resolution
    QString addressStr = listener->getHostname();

    if (addressStr.isEmpty()) {
        addressStr = listener->getHostAddress().toString();
        if (!addressStr.isEmpty()) {
            ui->peerAddrLineEdit->blockSignals(true);
            ui->peerAddrLineEdit->setText(addressStr);
            ui->peerAddrLineEdit->blockSignals(false);
            ui->hostResolvComboBox->blockSignals(true);
            ui->hostResolvComboBox->addItem(addressStr);
            ui->hostResolvComboBox->blockSignals(false);
        }
    }
    else { // we get dns resolution in this case
        ui->peerAddrLineEdit->setText(addressStr);
    }
}

IPNetworkClientWidget::~IPNetworkClientWidget()
{
    delete ui;
}

void IPNetworkClientWidget::onTlsToggled(bool enabled)
{
    // for easier use only
    if (enabled && ui->portSpinBox->value() == 80) {
        ui->portSpinBox->setValue(443);
    } else if (! enabled && ui->portSpinBox->value() == 443) {
        ui->portSpinBox->setValue(80);
    }
}

void IPNetworkClientWidget::onPortChanged(int value)
{
    listener->setHostPort((quint16) value);
}

void IPNetworkClientWidget::onIPChanged(QString value)
{
    listener->setHostAddress(QHostAddress(value));
}

void IPNetworkClientWidget::onTargetchanged()
{
    resolutionTimer.start();
    ui->hostResolvComboBox->clear();
}

void IPNetworkClientWidget::onResolutionTimerExpired()
{
    QString value = ui->peerAddrLineEdit->text();
    QHostAddress addr;
    if (addr.setAddress(value)) { // that's a valid IP
        ui->hostResolvComboBox->addItem(value);
        ui->hostResolvComboBox->setCurrentIndex(0);
        ui->hostResolvComboBox->setDisabled(true);
    } else { // that's not
        QHostInfo::lookupHost(value,this, SLOT(lookupFinished(QHostInfo)));
    }
}

void IPNetworkClientWidget::lookupFinished(QHostInfo info)
{
    if (info.error() != QHostInfo::NoError) {
        emit log(info.errorString(), "", Pip3lineConst::LERROR);
    } else { // success populating the combobox, at this point the list shoud not be empty ...
        QList<QHostAddress> list = info.addresses();
        if (list.isEmpty()) {
            qCritical() << "[NetworkClientWidget::lookupFinished] returned adress list is empty T_T";
            return;
        }

        for (int i = 0; i < list.size(); i++) {
            ui->hostResolvComboBox->addItem(list.at(i).toString());
        }
        ui->hostResolvComboBox->setCurrentIndex(0);
        ui->hostResolvComboBox->setDisabled(false);
        listener->setHostAddress(list.at(0));
        listener->setHostname(info.hostName());
        emit newSNI(info.hostName());
    }
}
