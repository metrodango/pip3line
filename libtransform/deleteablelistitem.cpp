/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "deleteablelistitem.h"
#include "ui_deleteablelistitem.h"
#include <QDebug>

DeleteableListItem::DeleteableListItem(const QString &text, const QPixmap &im, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::DeleteableListItem();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::DeleteableListItem X{");
    }
    ui->setupUi(this);
    ui->textLabel->setText(text);

    if (!im.isNull())
        ui->pixLabel->setPixmap(im);
    connect(ui->deletePushButton, SIGNAL(clicked()), this, SLOT(onDelete()));
}

DeleteableListItem::~DeleteableListItem()
{
   // qDebug() << "D3stroying " << this;
    delete ui;
}

QString DeleteableListItem::getName()
{
    return ui->textLabel->text();
}

void DeleteableListItem::setEnableDelete(bool val)
{
    ui->deletePushButton->setEnabled(val);
}


void DeleteableListItem::onDelete()
{
    emit itemDeleted(ui->textLabel->text());
    deleteLater();
}
