/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "floatingdialog.h"
#include "ui_floatingdialog.h"
#include <QCloseEvent>
#include <QDebug>

FloatingDialog::FloatingDialog(GuiHelper * guiHelper, QWidget * widget, QWidget *parent) :
    AppDialog(guiHelper, parent)
{
    allowReject = true;
    ui = new(std::nothrow) Ui::FloatingDialog();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::FloatingDialog X{");
    }
    ui->setupUi(this);
    setModal(false);
    ui->verticalLayout->addWidget(widget);
    widget->setVisible(true);
}

FloatingDialog::~FloatingDialog()
{
    delete ui;
}

void FloatingDialog::setAllowReject(bool val)
{
    allowReject = val;
}

void FloatingDialog::closeEvent(QCloseEvent *event)
{
    if (allowReject) {
        reject();
    }
    //emit hiding(); ??
    event->accept();

}
