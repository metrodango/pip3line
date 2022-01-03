#include "additionaluidialog.h"
#include "ui_additionaluidialog.h"

AdditionalUIDialog::AdditionalUIDialog(QWidget *childWidget, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdditionalUIDialog)
{
    ui->setupUi(this);
    childWidget->setParent(this);
    ui->verticalLayout->insertWidget(0,childWidget);
    connect(this, &QDialog::rejected, this, &QDialog::deleteLater);
}

AdditionalUIDialog::~AdditionalUIDialog()
{
    delete ui;
}
