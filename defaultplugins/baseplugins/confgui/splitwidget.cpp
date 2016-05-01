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
    if (ui == NULL) {
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

    connect(ui->charWidget,SIGNAL(charChanged(char)), SLOT(onSeparatorChange(char)));
    connect(ui->groupSpinBox,SIGNAL(valueChanged(int)), SLOT(onGroupChange(int)));
    connect(ui->allCheckBox,SIGNAL(toggled(bool)),  SLOT(onAllGroupChange(bool)));
    connect(ui->trimCheckBox,SIGNAL(toggled(bool)), SLOT(onTrimChange(bool)));
    connect(ui->procLineBLinecheckBox, SIGNAL(toggled(bool)), SLOT(onProcLineChanged(bool)));
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
