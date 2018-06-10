/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "numbertocharwidget.h"

#include "ui_numbertocharwidget.h"

NumberToCharWidget::NumberToCharWidget(NumberToChar *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::NumberToCharWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::NumberToCharWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->separatorLineEdit->setText(QByteArray(1,transform->getSeparator()));

    connect(ui->separatorLineEdit, &QLineEdit::textEdited, this, &NumberToCharWidget::onSeparatorChanged);
}

NumberToCharWidget::~NumberToCharWidget()
{
    delete ui;
}

void NumberToCharWidget::onSeparatorChanged(const QString &val)
{
    if (val.size() == 1) { // one and only character
        QByteArray c = val.toUtf8();
        if (c.size() > 1) { // ignoring multi-bytes characters
            ui->separatorLineEdit->clear();
            return;
        }
        char ch = c.at(0);
        if (!transform->setSeparator(ch)) {
            ui->separatorLineEdit->clear();
        }
    }
}


