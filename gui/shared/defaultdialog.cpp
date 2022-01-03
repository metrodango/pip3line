#include "defaultdialog.h"
#include "ui_defaultdialog.h"

DefaultDialog::DefaultDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DefaultDialog)
{
    ui->setupUi(this);
    setModal(false);
}

DefaultDialog::~DefaultDialog()
{
    delete ui;
}

void DefaultDialog::setMainWidget(QWidget *widget)
{
    ui->label->setVisible(false);
    ui->verticalLayout->insertWidget(0,widget);
}
