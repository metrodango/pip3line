#include "dtlsclientlistenerwidget.h"
#include "ui_dtlsclientlistenerwidget.h"
#include "dtlsclientlistener.h"

DtlsClientListenerWidget::DtlsClientListenerWidget(DTLSClientListener * blocksource, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DtlsClientListenerWidget),
    blocksource(blocksource)
{
    ui->setupUi(this);
    ui->useSrcPortCheckBox->setChecked(blocksource->getUseSrcPorts());

    connect(ui->useSrcPortCheckBox, &QCheckBox::toggled, this, &DtlsClientListenerWidget::onSrcPortUseToggle);
}

DtlsClientListenerWidget::~DtlsClientListenerWidget()
{
    delete ui;
    blocksource = nullptr;
}

void DtlsClientListenerWidget::onSrcPortUseToggle(bool value)
{
    blocksource->setUseSrcPorts(value);
}
