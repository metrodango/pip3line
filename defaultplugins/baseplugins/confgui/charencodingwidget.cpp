/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "charencodingwidget.h"
#include "ui_charencodingwidget.h"
#include <QTextCodec>

CharEncodingWidget::CharEncodingWidget(CharEncoding *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::CharEncodingWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::CharEncodingWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    QList<QByteArray> codecs =  QTextCodec::availableCodecs();
    qSort(codecs);
    for (int i = 0; i < codecs.size(); i++) {
        ui->codecComboBox->addItem(QString(codecs.at(i)),QVariant(codecs.at(i)));
    }
    ui->codecComboBox->setCurrentIndex(ui->codecComboBox->findData((QVariant) transform->getCodecName()));

    ui->convertInvalidToNullCheckBox->setChecked(transform->getConvertInvalidToNull());
    ui->insertBOMCheckBox->setChecked(transform->getIncludeHeader());

    connect(ui->codecComboBox,SIGNAL(currentIndexChanged(int)), this, SLOT(onCodecChange()));
    connect(ui->UTF8pushButton, SIGNAL(clicked()), SLOT(onUTF8Clicked()));
}

CharEncodingWidget::~CharEncodingWidget()
{
    delete ui;
}

void CharEncodingWidget::onCodecChange()
{
    transform->setCodecName(ui->codecComboBox->itemData(ui->codecComboBox->currentIndex()).toByteArray());
}

void CharEncodingWidget::on_UTF16PushButton_clicked()
{
    transform->setCodecName("UTF-16");
    ui->codecComboBox->setCurrentIndex(ui->codecComboBox->findData((QVariant) transform->getCodecName()));
}

void CharEncodingWidget::on_latin1PushButton_clicked()
{
    transform->setCodecName("ISO-8859-1");
    ui->codecComboBox->setCurrentIndex(ui->codecComboBox->findData((QVariant) transform->getCodecName()));
}

void CharEncodingWidget::onUTF8Clicked()
{
    transform->setCodecName("UTF-8");
    ui->codecComboBox->setCurrentIndex(ui->codecComboBox->findData((QVariant) transform->getCodecName()));
}

void CharEncodingWidget::onInsertBOMChanged(bool val)
{
    transform->setIncludeHeader(val);
}

void CharEncodingWidget::onConvertInvalidToNullChanged(bool val)
{
    transform->setConvertInvalidToNull(val);
}
