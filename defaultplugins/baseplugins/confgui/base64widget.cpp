/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "base64widget.h"
#include "ui_base64widget.h"

Base64Widget::Base64Widget(Base64 *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::Base64Widget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::Base64Widget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->variantComboBox->addItems(Base64::VARIATIONS);
    ui->variantComboBox->setCurrentIndex(transform->getChoosenVariation());
    ui->char62LineEdit->setText(QString(transform->getChar62()));
    ui->char63LineEdit->setText(QString(transform->getChar63()));
    ui->paddingLineEdit->setText(QString(transform->getPaddingChar()));

    if (transform->getChoosenVariation() == Base64::STANDARD) {
        ui->inclPaddingRadioButton->setChecked(true);
    } else if (transform->getChoosenVariation() == Base64::SAFEURL) {
        ui->noPaddingRadioButton->setChecked(true);
    } else if (transform->getChoosenVariation() == Base64::DOTNET) {
        ui->netPaddingRadioButton->setChecked(true);
    } else if (transform->getPaddingType() == Base64::DEFAULTPADDING) {
        ui->inclPaddingRadioButton->setChecked(true);
    }else if (transform->getPaddingType() == Base64::DOTNETPADDING) {
        ui->netPaddingRadioButton->setChecked(true);
    } else {
        ui->noPaddingRadioButton->setChecked(true);
    }

    connect(ui->char62LineEdit, &QLineEdit::textChanged, this, &Base64Widget::verifyChar62);
    connect(ui->char63LineEdit, &QLineEdit::textChanged, this, &Base64Widget::verifyChar63);
    connect(ui->paddingLineEdit, &QLineEdit::textChanged, this, &Base64Widget::verifyCharPadding);
    connect(ui->inclPaddingRadioButton, &QRadioButton::clicked, this, &Base64Widget::onCustomConf);
    connect(ui->netPaddingRadioButton, &QRadioButton::clicked, this, &Base64Widget::onCustomConf);
    connect(ui->noPaddingRadioButton, &QRadioButton::clicked, this, &Base64Widget::onCustomConf);
    //connect(ui->variantComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &Base64Widget::onChangeVariant);
    connect(ui->variantComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeVariant(int)));
}

Base64Widget::~Base64Widget()
{
    delete ui;
}

void Base64Widget::onChangeVariant(int variant) {
    ui->char62LineEdit->blockSignals(true);
    ui->char63LineEdit->blockSignals(true);
    ui->paddingLineEdit->blockSignals(true);
    ui->inclPaddingRadioButton->blockSignals(true);
    ui->netPaddingRadioButton->blockSignals(true);
    ui->noPaddingRadioButton->blockSignals(true);

    transform->setChoosenVariation((Base64::Variation) variant);
    if (variant != Base64::CUSTOM) {
        if (variant == Base64::STANDARD) {
            ui->inclPaddingRadioButton->setChecked(true);
        } else if (variant == Base64::SAFEURL) {
            ui->noPaddingRadioButton->setChecked(true);
        } else if (variant == Base64::DOTNET) {
            ui->netPaddingRadioButton->setChecked(true);
        }

        ui->char62LineEdit->setText(QString(transform->getChar62()));
        ui->char63LineEdit->setText(QString(transform->getChar63()));
        ui->paddingLineEdit->setText(QString(transform->getPaddingChar()));
    }

    ui->char62LineEdit->blockSignals(false);
    ui->char63LineEdit->blockSignals(false);
    ui->paddingLineEdit->blockSignals(false);
    ui->inclPaddingRadioButton->blockSignals(false);
    ui->netPaddingRadioButton->blockSignals(false);
    ui->noPaddingRadioButton->blockSignals(false);
}

void Base64Widget::verifyChar62(QString nchar)
{
    ui->variantComboBox->setCurrentIndex(Base64::CUSTOM);
    if (nchar.isEmpty())
        return;
    QByteArray c = nchar.toUtf8();
    if (c.size() > 1) {
        ui->char62LineEdit->clear();
        return;
    }
    char ch = c.at(0);

    if ( !transform->setChar62(ch) ) {
        ui->char62LineEdit->clear();
    }
}

void Base64Widget::verifyChar63(QString nchar)
{
    ui->variantComboBox->setCurrentIndex(Base64::CUSTOM);
    transform->setChoosenVariation(Base64::CUSTOM);
    if (nchar.isEmpty())
        return;

    QByteArray c = nchar.toUtf8();
    if (c.size() > 1) {
        ui->char63LineEdit->clear();
        return;
    }
    char ch = c.at(0);

    if (!transform->setChar63(ch)) {
        ui->char63LineEdit->clear();
    }
}

void Base64Widget::verifyCharPadding(QString nchar)
{
    ui->variantComboBox->setCurrentIndex(Base64::CUSTOM);
    if (nchar.isEmpty())
        return;

    QByteArray c = nchar.toUtf8();
    if (c.size() > 1) {
        ui->paddingLineEdit->clear();
        return;
    }
    char ch = c.at(0);

    if (!transform->setPaddingChar(ch)) {
        ui->paddingLineEdit->clear();
    }
}

void Base64Widget::onCustomConf()
{
    transform->setChoosenVariation(Base64::CUSTOM);
    ui->variantComboBox->setCurrentIndex(Base64::CUSTOM);
    if (ui->inclPaddingRadioButton->isChecked())
        transform->setPaddingType(Base64::DEFAULTPADDING);
    else if (ui->netPaddingRadioButton->isChecked())
        transform->setPaddingType(Base64::DOTNETPADDING);
    else
        transform->setPaddingType(Base64::NOPADDING);

}
