/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "cutwidget.h"
#include "ui_cutwidget.h"

CutWidget::CutWidget(Cut *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::CutWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::CutWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->fromSpinBox->setValue(transform->getFromPos());
    ui->lengthSpinBox->setValue(transform->getLength());
    ui->everythingCheckBox->setChecked(transform->doCutEverything());
    ui->cutRadioButton->setChecked(transform->isClassicCut());
    ui->lineByLineCheckBox->setChecked(transform->isLineByLine());

    //connect(ui->fromSpinBox, qOverload<int>(&QSpinBox::valueChanged),this, &CutWidget::onFromChange);
    connect(ui->fromSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onFromChange(int)));
    //connect(ui->lengthSpinBox,qOverload<int>(&QSpinBox::valueChanged),this, &CutWidget::onLengthChange);
    connect(ui->lengthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onLengthChange(int)));
    connect(ui->everythingCheckBox, &QCheckBox::toggled, this, &CutWidget::onEverythingChange);
    connect(ui->cutRadioButton, &QRadioButton::toggled, this, &CutWidget::onCutToggled);
    connect(ui->lineByLineCheckBox, &QCheckBox::toggled, this, &CutWidget::onLineByLineToggled);
}

CutWidget::~CutWidget()
{
    delete ui;
}


void CutWidget::onFromChange(int value)
{
    if (!transform->setFromPos(value)) {
        ui->lengthSpinBox->blockSignals(true);
        ui->lengthSpinBox->setValue(transform->getLength());
        ui->lengthSpinBox->blockSignals(false);
    }
}

void CutWidget::onLengthChange(int value)
{
    if (!transform->setLength(value)) {
        ui->lengthSpinBox->blockSignals(true);
        ui->lengthSpinBox->setValue(transform->getLength());
        ui->lengthSpinBox->blockSignals(false);
    } else {
        ui->everythingCheckBox->blockSignals(true);
        ui->everythingCheckBox->setChecked(false);
        ui->everythingCheckBox->blockSignals(false);
    }
}

void CutWidget::onEverythingChange(bool value)
{
    transform->setCutEverything(value);
}

void CutWidget::onCutToggled(bool checked)
{
    ui->fromSpinBox->setEnabled(checked);
    ui->everythingCheckBox->setEnabled(checked);
    transform->setClassicCut(checked);
}

void CutWidget::onLineByLineToggled(bool checked)
{
    transform->setLineByLine(checked);
}
