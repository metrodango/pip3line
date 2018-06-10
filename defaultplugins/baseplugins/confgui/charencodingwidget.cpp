/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "charencodingwidget.h"
#include "ui_charencodingwidget.h"
#include <QTextCodec>
#include <QDebug>

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
    std::sort(codecs.begin(), codecs.end());

    for (int i = 0; i < codecs.size(); i++) {
        ui->codecComboBox->addItem(QString(codecs.at(i)),QVariant(codecs.at(i)));
    }
    ui->codecComboBox->setCurrentIndex(ui->codecComboBox->findData((QVariant) transform->getCodecName()));

    ui->convertInvalidToNullCheckBox->setChecked(transform->getConvertInvalidToNull());
    ui->insertBOMCheckBox->setChecked(transform->getIncludeHeader());

//    connect(ui->codecComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int) {
//        transform->setCodecName(ui->codecComboBox->itemData(ui->codecComboBox->currentIndex()).toByteArray());
//    });
    connect(ui->codecComboBox, SIGNAL(currentIndexChanged(int)), this , SLOT(onCodecChanged()));
    connect(ui->UTF8pushButton, &QPushButton::clicked, this, &CharEncodingWidget::onUTF8Clicked);
    connect(ui->insertBOMCheckBox, &QCheckBox::toggled, this , [=](bool val) {transform->setIncludeHeader(val);});
    connect(ui->convertInvalidToNullCheckBox, &QCheckBox::toggled, this , [=](bool val) {transform->setConvertInvalidToNull(val);});
}

CharEncodingWidget::~CharEncodingWidget()
{
    delete ui;
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

void CharEncodingWidget::onCodecChanged()
{
    transform->setCodecName(ui->codecComboBox->itemData(ui->codecComboBox->currentIndex()).toByteArray());
}
