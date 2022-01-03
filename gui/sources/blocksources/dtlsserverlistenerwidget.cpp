#include "dtlsserverlistenerwidget.h"
#include "ui_dtlsserverlistenerwidget.h"
#include "dtlsserverlistener.h"

DtlsServerListenerWidget::DtlsServerListenerWidget(DtlsServerListener *blocksource, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DtlsServerListenerWidget),
    blocksource(blocksource)
{
    ui->setupUi(this);

    ui->enableClientVerificationCheckBox->setChecked(blocksource->getDtlsVerificationEnabled());

    connect(ui->enableClientVerificationCheckBox, &QCheckBox::toggled, this, &DtlsServerListenerWidget::onEnableClientVerifiation);
}

DtlsServerListenerWidget::~DtlsServerListenerWidget()
{
    delete ui;
}

void DtlsServerListenerWidget::onEnableClientVerifiation(bool value)
{
    blocksource->setDtlsVerificationEnabled(value);
}
