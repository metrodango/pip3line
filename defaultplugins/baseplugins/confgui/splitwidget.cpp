/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "splitwidget.h"
#include "ui_splitwidget.h"

SplitWidget::SplitWidget(Split *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::SplitWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::SplitWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->charWidget->setChar(transform->getSeparator());

    ui->groupSpinBox->setValue(transform->getSelectedGroup());
    ui->groupSpinBox->setMaximum(Split::MAXGROUPVALUE);
    ui->groupSpinBox->setMinimum(0);
    ui->allCheckBox->setChecked(transform->doWeTakeAllGroup());
    ui->trimCheckBox->setChecked(transform->isTrimmingBlank());
    ui->procLineBLinecheckBox->setChecked(transform->isProcessingLineByLine());

    connect(ui->charWidget, &HexWidget::charChanged, this, &SplitWidget::onSeparatorChange);
    //connect(ui->groupSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &SplitWidget::onGroupChange);
    connect(ui->groupSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onGroupChange(int)));
    connect(ui->allCheckBox, &QCheckBox::toggled, this, &SplitWidget::onAllGroupChange);
    connect(ui->trimCheckBox, &QCheckBox::toggled, this, &SplitWidget::onTrimChange);
    connect(ui->procLineBLinecheckBox, &QCheckBox::toggled, this, &SplitWidget::onProcLineChanged);
}

SplitWidget::~SplitWidget()
{
    delete ui;
}

void SplitWidget::onGroupChange(int val)
{
    if (!transform->setSelectedGroup(val)) {
        ui->groupSpinBox->setValue(transform->getSelectedGroup());
    } else {
        ui->allCheckBox->blockSignals(true);
        ui->allCheckBox->setChecked(false);
        ui->allCheckBox->blockSignals(false);
    }
}

void SplitWidget::onAllGroupChange(bool val)
{
    transform->setTakeAllGroup(val);
}

void SplitWidget::onTrimChange(bool val)
{
    transform->setTrimBlank(val);
}

void SplitWidget::onProcLineChanged(bool val)
{
    transform->setProcessLineByLine(val);
}

void SplitWidget::onSeparatorChange(char val)
{
    transform->setSeparator(val);
}
