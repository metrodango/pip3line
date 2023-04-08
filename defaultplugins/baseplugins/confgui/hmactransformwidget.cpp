/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "hmactransformwidget.h"
#include "../hmactransform.h"
#include "ui_hmactransformwidget.h"
#include <QMapIterator>
#include <QLineEdit>

HMACTransformWidget::HMACTransformWidget(HMACTransform *ntranform, QWidget *parent):
    QWidget(parent),
    transform(ntranform)
{
    ui = new(std::nothrow) Ui::HMACTransformWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::HMACTransformWidget X{");
    }
    ui->setupUi(this);

    QMapIterator<int,QString> i(transform->hashes);


     while (i.hasNext()) {
         i.next();
         ui->hashComboBox->addItem(i.value());
     }

     ui->hashComboBox->setCurrentIndex(transform->getSelectedHash());
     QString key = QString::fromUtf8(transform->getKey().toHex());
     if (!key.isEmpty()) {
        ui->keyComboBox->addItem(key);
        ui->keyComboBox->setCurrentIndex(0);
     }

     connect(ui->hashComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onHashChanged(int)));
     //connect(ui->hashComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int index) {transform->setSelectedHash(index);});
     connect(ui->keyComboBox->lineEdit(), &QLineEdit::returnPressed, this, &HMACTransformWidget::onKeyLineReturn);
}

HMACTransformWidget::~HMACTransformWidget()
{
    delete ui;
}

void HMACTransformWidget::onHashChanged(int val)
{
    transform->setSelectedHash(val);
}

void HMACTransformWidget::onKeyLineReturn()
{
    QString key = ui->keyComboBox->currentText();
    if (!key.isEmpty()) {
        transform->setKey(QByteArray::fromHex(key.toUtf8()));
        ui->keyComboBox->insertItem(0,key);
    }
}
