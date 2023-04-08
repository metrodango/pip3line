/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "base32widget.h"
#include "ui_base32widget.h"
#include <QDebug>

Base32Widget::Base32Widget(Base32 *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::Base32Widget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::Base32Widget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->variantComboBox->addItems(Base32::VARIATIONS);
    ui->paddingCheckBox->setChecked(transform->isPaddingIncluded());
    ui->paddingLineEdit->setText(QString(transform->getPaddingChar()));
    ui->variantComboBox->setCurrentIndex(transform->getVariant());
    ui->charsetDisplay->setText(QString(transform->getCharTable()));

    //connect(ui->variantComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &Base32Widget::onVariantchange);
    connect(ui->variantComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onVariantchange(int)));
    connect(ui->paddingCheckBox, &QCheckBox::toggled, this, [=] (bool val) { transform->setIncludePadding(val);});
    connect(ui->paddingLineEdit, &QLineEdit::textEdited, this, &Base32Widget::onPaddingChange);
}

Base32Widget::~Base32Widget()
{
    delete ui;
}

void Base32Widget::onVariantchange(int newVariant)
{
    transform->setVariant(static_cast<Base32::CharSetVariant>(newVariant));
    ui->charsetDisplay->setText(QString(transform->getCharTable()));
}

void Base32Widget::onPaddingChange(QString val)
{
    //qDebug() << val;
    if (val.size() == 1) { // one and only character
        QByteArray c = val.toUtf8();
        if (c.size() > 1) { // ignoring multi-bytes characters
            ui->paddingLineEdit->clear();
            return;
        }
        char ch = c.at(0);

        if (!transform->setPaddingChar(ch)) {
            ui->paddingLineEdit->clear();
        } else {
            ui->paddingCheckBox->setChecked(true);
        }
    } else {
        ui->paddingCheckBox->setChecked(false);
    }
}

