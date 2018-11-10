#include "statedialog.h"
#include "ui_statedialog.h"
#include <QRect>
#include <QDesktopWidget>
#include <QScreen>
#include <QGuiApplication>

StateDialog::StateDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    ui(new Ui::StateDialog)
{
    ui->setupUi(this);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);

    const QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    move(screen.center() - this->rect().center());
}

StateDialog::~StateDialog()
{
    delete ui;
}

void StateDialog::log(QString message, QString , Pip3lineConst::LOGLEVEL )
{
    ui->logLabel->setText(message);
    update();
}

void StateDialog::update()
{
    ui->progressBar->setValue((ui->progressBar->value() + 1));
}
