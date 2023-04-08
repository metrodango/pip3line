/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "textinputdialog.h"
#include "ui_textinputdialog.h"

TextInputDialog::TextInputDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint)
{
    ui = new(std::nothrow) Ui::textInputDialog;
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::textInputDialog X{");
    }
    ui->setupUi(this);
}

TextInputDialog::~TextInputDialog()
{
    delete ui;
}

void TextInputDialog::setTextLabel(QString text)
{
    ui->inputLabel->setText(text);
}

void TextInputDialog::setPixLabel(QPixmap pix)
{
    ui->inputLabel->setPixmap(pix);
}

QString TextInputDialog::getInputText() const
{
    return ui->inputLineEdit->text();
}

void TextInputDialog::setText(QString text)
{
    ui->inputLineEdit->setText(text);
    ui->inputLineEdit->selectAll();
}


