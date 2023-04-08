/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "distormwidget.h"
#include "ui_distormwidget.h"

const QStringList DistormWidget::asmTypeStrings = QStringList() << "16 Bits" << "32 Bits" << "64 Bits";

DistormWidget::DistormWidget(DistormTransf *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::DistormWidget();
    if (ui == nullptr)
        qFatal("Cannot allocate memory for Ui::DistormWidget X{");

    transform = ntransform;
    ui->setupUi(this);
    ui->asmTypeComboBox->addItems(asmTypeStrings);
    switch(transform->getDecodeType()) {
        case DistormTransf::A16Bits:
            ui->asmTypeComboBox->setCurrentIndex(0);
            break;
        case DistormTransf::A32Bits:
            ui->asmTypeComboBox->setCurrentIndex(1);
            break;
        case DistormTransf::A64Bits:
            ui->asmTypeComboBox->setCurrentIndex(2);
            break;
    }

    ui->offsetLineEdit->setText(QString::number(transform->getOffset(),16)); // hex format
    ui->showOffsetCheckBox->setChecked(transform->getShowOffset());
    ui->showOpcodesCheckBox->setChecked(transform->getShowOpcodes());

    //connect(ui->maxInstructionSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &DistormWidget::onMaxInstruc);
    connect(ui->maxInstructionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMaxInstruc(int)));
    connect(ui->offsetLineEdit, &QLineEdit::textChanged, this, &DistormWidget::onOffset);
    //connect(ui->asmTypeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &DistormWidget::onType);
    connect(ui->asmTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onType(int)));
    connect(ui->showOffsetCheckBox, &QCheckBox::toggled, this, &DistormWidget::onShowOffset);
    connect(ui->showOpcodesCheckBox, &QCheckBox::toggled, this, &DistormWidget::onShowOpcodes);

}

DistormWidget::~DistormWidget()
{
    delete ui;
}

void DistormWidget::onMaxInstruc(int val)
{
    transform->setMaxInstruction(static_cast<uint>(val));
}

void DistormWidget::onOffset(QString val)
{
    quint64 realVal = 0;
    bool ok;
    realVal = val.toULongLong(&ok, 16);
    if (!ok) {
        ui->offsetLineEdit->setStyleSheet("QLineEdit { border: 1px ridge red }");
    } else {
        ui->offsetLineEdit->setStyleSheet("");
        transform->setOffset(realVal);
    }
}

void DistormWidget::onType(int val)
{
    if (val == 0) {
        transform->setDecodeType(DistormTransf::A16Bits);
    } else if (val == 1) {
        transform->setDecodeType(DistormTransf::A32Bits);
    } else if (val == 2) {
        transform->setDecodeType(DistormTransf::A64Bits);
    }
}

void DistormWidget::onShowOffset(bool val)
{
    transform->setShowOffset(val);
}

void DistormWidget::onShowOpcodes(bool val)
{
    transform->setShowOpcodes(val);
}
