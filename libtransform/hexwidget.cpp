/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "hexwidget.h"
#include "ui_hexwidget.h"
#include <transformabstract.h>
#include <QMessageBox>
#include <QDebug>

CharValidator::CharValidator(QObject *parent) :
    QValidator(parent)
{

}

QValidator::State CharValidator::validate(QString &input, int &) const
{
    if (input.isEmpty())
        return QValidator::Intermediate;

    QByteArray bvalue = input.toUtf8();
    if (bvalue.size() > 1) {
        return QValidator::Invalid;
    }

    return QValidator::Acceptable;
}

HexWidget::HexWidget(QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::HexWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::HexWidget X{");
    }
    ui->setupUi(this);

    CharValidator *validator = new(std::nothrow) CharValidator(this);
    if (validator == nullptr) {
        qFatal("Cannot allocate memory for CharValidator X{");
    }
    ui->charLineEdit->setValidator(validator);

    connect(ui->hexLineEdit, &QLineEdit::textEdited,this, &HexWidget::onHexChanged);
    connect(ui->charLineEdit, &QLineEdit::textEdited, this, &HexWidget::onCharChanged);
    setChar('\00');
}

HexWidget::~HexWidget()
{
    delete ui;
}

char HexWidget::getChar()
{
    QByteArray input = QByteArray::fromHex(ui->hexLineEdit->text().toUtf8());
    if (!input.isEmpty()) {
        return input.at(0);
    }
    return '\00';
}

void HexWidget::setChar(char c)
{
    QString data;
    QString charData;
    QString hex = QString::fromUtf8(QByteArray(1,c).toHex());

    if (TransformAbstract::isPrintable((qint32)c)) {
        data = QString::fromUtf8(&c, 1);
        charData = data;
    } else {
        switch (c) {
            case '\n':
                data = "\\n";
                break;
            case '\r':
                data = "\\r";
                break;
            default:
                data = tr("\\%1").arg(hex);
        }
    }

    ui->valueLabel->setText(data);

    ui->hexLineEdit->blockSignals(true);
    ui->hexLineEdit->setText(hex);
    ui->hexLineEdit->setFocus();
    ui->hexLineEdit->selectAll();
    ui->hexLineEdit->blockSignals(false);

    ui->charLineEdit->blockSignals(true);
    ui->charLineEdit->setText(charData);
    ui->charLineEdit->blockSignals(false);
}

void HexWidget::onHexChanged()
{
    QByteArray input = QByteArray::fromHex(ui->hexLineEdit->text().toUtf8());
    QString data;
    QString charData;

    if (!input.isEmpty()) {
        char c = input.at(0);
        if (TransformAbstract::isPrintable((qint32)c)) {
            data = QString::fromUtf8(&c, 1);
            charData = data;
        } else {
            switch (c) {
                case '\n':
                    data = "\\n";
                    break;
                case '\r':
                    data = "\\r";
                    break;
                default:
                    data = tr("\\%1").arg(ui->hexLineEdit->text());
            }
        }

        emit charChanged(c);
    }
    ui->valueLabel->setText(data);
    ui->charLineEdit->blockSignals(true);
    ui->charLineEdit->setText(charData);
    ui->charLineEdit->blockSignals(false);
}

void HexWidget::onCharChanged(QString val)
{
    if (val.size() > 1) {
        qCritical() << tr("[HexWidget::onCharChanged] string size is > 1, really ?? T_T");
    }

    QByteArray bvalue = val.toUtf8();
    if (bvalue.size() != 1) {
        return;
    }
    QString hex = QString::fromUtf8(bvalue.toHex());

    ui->hexLineEdit->blockSignals(true);
    ui->hexLineEdit->setText(hex);
    ui->hexLineEdit->selectAll();
    ui->hexLineEdit->blockSignals(false);
    ui->valueLabel->setText(val);

    emit charChanged(bvalue.at(0));
}
