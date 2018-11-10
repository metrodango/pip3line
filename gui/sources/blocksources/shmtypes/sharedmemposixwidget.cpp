#include "sharedmemposixwidget.h"
#include "ui_sharedmemposixwidget.h"
#include "sharedmemposix.h"

SharedMemPosixWidget::SharedMemPosixWidget(SharedMemPosix *connector, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SharedMemPosixWidget),
    connector(connector)
{
    ui->setupUi(this);

    ui->keyLineEdit->setText(connector->getKey());
    ui->sizeSpinBox->setValue(connector->getSize());
    connect(ui->submitPushButton, &QPushButton::clicked, this, &SharedMemPosixWidget::onSubmit);
}

SharedMemPosixWidget::~SharedMemPosixWidget()
{
    delete ui;
}

void SharedMemPosixWidget::onSubmit()
{
    connector->setKey(ui->keyLineEdit->text());
    connector->setSize(ui->sizeSpinBox->value());
}
