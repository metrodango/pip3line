/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
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

    connect(ui->pushButton, &QPushButton::clicked, this, [=](bool){transform->reRandomize();});
}

RandomCaseWidget::~RandomCaseWidget()
{
    delete ui;
}
