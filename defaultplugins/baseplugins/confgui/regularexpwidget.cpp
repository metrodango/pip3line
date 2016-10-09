/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "regularexpwidget.h"
#include "ui_regularexpwidget.h"

RegularExpWidget::RegularExpWidget(RegularExp *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::RegularExpWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::RegularExpWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);

    ui->lineByLineCheckBox->setChecked(transform->doWeProcessLineByLine());
    ui->typeComboBox->setCurrentIndex(transform->getActionType());
    ui->typeComboBox->installEventFilter(this);
    ui->actionStackedWidget->setCurrentIndex(transform->getActionType());
    ui->greedyCheckBox->setChecked(transform->isUsingGreedyQuantifier());
    ui->regExpLineEdit->setText(transform->getRegularExpression());
    ui->caseCheckBox->setChecked(transform->isCaseInsensitive());
    ui->groupSpinBox->setValue(transform->getSelectedGroupIndex());
    ui->allGroupCheckBox->setChecked(transform->isTakingAllGroup());
    ui->groupSpinBox->setEnabled(!transform->isTakingAllGroup());
    ui->replaceLineEdit->setText(transform->getReplacementString());

    connect(ui->lineByLineCheckBox,SIGNAL(toggled(bool)), this, SLOT(onLinByLineChange(bool)));
    connect(ui->typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onActionTypeChange(int)));
    connect(ui->greedyCheckBox, SIGNAL(toggled(bool)), this, SLOT(onGreedyChange(bool)));
    connect(ui->regExpLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onRegularExpressionChange(QString)));
    connect(ui->caseCheckBox, SIGNAL(toggled(bool)), this, SLOT(onCaseInsensitiveChange(bool)));
    connect(ui->groupSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onGroupChange(int)));
    connect(ui->allGroupCheckBox, SIGNAL(toggled(bool)), this, SLOT(onAllGroupsChange(bool)));
    connect(ui->replaceLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onReplacementStringChange(QString)));
}

RegularExpWidget::~RegularExpWidget()
{
    delete ui;
}

void RegularExpWidget::onActionTypeChange(int val)
{
    transform->setActionType((RegularExp::Actions)val);
    ui->actionStackedWidget->setCurrentIndex((RegularExp::Actions)val);
}

void RegularExpWidget::onGreedyChange(bool val)
{
    transform->setUsingGreedyQuantifier(val);
}

void RegularExpWidget::onRegularExpressionChange(QString val)
{
    transform->setExpression(val);
}

void RegularExpWidget::onCaseInsensitiveChange(bool val)
{
    transform->setCaseInsensitive(val);
}

void RegularExpWidget::onGroupChange(int val)
{
    transform->setSelectedGroup(val);
}

void RegularExpWidget::onAllGroupsChange(bool val)
{
    ui->groupSpinBox->setEnabled(!val);

    transform->setAllGroups(val);
}

void RegularExpWidget::onReplacementStringChange(QString val)
{
    transform->setReplacementString(val);
}

void RegularExpWidget::onLinByLineChange(bool val)
{
    transform->setProcessLineByLine(val);
}

bool RegularExpWidget::eventFilter(QObject *o, QEvent *e)
{
    // Filtering out wheel event for comboboxes
    if ( e->type() == QEvent::Wheel && qobject_cast<QComboBox*>(o) )
    {
        e->ignore();
        return true;
    }
    return false;
}
