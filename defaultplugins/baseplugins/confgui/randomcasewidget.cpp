/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "randomcasewidget.h"
#include "ui_randomcasewidget.h"
#include <QDebug>

RandomCaseWidget::RandomCaseWidget(RandomCase *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::RandomCaseWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::RandomCaseWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
}

RandomCaseWidget::~RandomCaseWidget()
{
    delete ui;
}

void RandomCaseWidget::onRerandomize()
{
    transform->reRandomize();
}
