#include "statestatuswidget.h"
#include "ui_statestatuswidget.h"

StateStatusWidget::StateStatusWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StateStatusWidget)
{
    ui->setupUi(this);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat(tr("Saving/restore"));
}

StateStatusWidget::~StateStatusWidget()
{
    delete ui;
}

void StateStatusWidget::log(QString , QString , Pip3lineConst::LOGLEVEL )
{
    update();
}

void StateStatusWidget::update()
{
    ui->progressBar->setValue((ui->progressBar->value() + 1));
}

void StateStatusWidget::setMessage(QString message)
{
    ui->progressBar->setFormat(message);
}
