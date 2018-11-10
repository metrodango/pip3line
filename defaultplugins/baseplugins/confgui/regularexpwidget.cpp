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

    connect(ui->lineByLineCheckBox, &QCheckBox::toggled, this, &RegularExpWidget::onLinByLineChange);
    //connect(ui->typeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &RegularExpWidget::onActionTypeChange);
    connect(ui->typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onActionTypeChange(int)));
    connect(ui->greedyCheckBox, &QCheckBox::toggled, this, &RegularExpWidget::onGreedyChange);
    connect(ui->regExpLineEdit, &QLineEdit::textChanged, this, &RegularExpWidget::onRegularExpressionChange);
    connect(ui->caseCheckBox, &QCheckBox::toggled, this, &RegularExpWidget::onCaseInsensitiveChange);
    //connect(ui->groupSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &RegularExpWidget::onGroupChange);
    connect(ui->groupSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onGroupChange(int)));
    connect(ui->allGroupCheckBox, &QCheckBox::toggled, this, &RegularExpWidget::onAllGroupsChange);
    connect(ui->replaceLineEdit, &QLineEdit::textChanged, this, &RegularExpWidget::onReplacementStringChange);
}

RegularExpWidget::~RegularExpWidget()
{
    delete ui;
}

void RegularExpWidget::onActionTypeChange(int val)
{
    transform->setActionType(static_cast<RegularExp::Actions>(val));
    ui->actionStackedWidget->setCurrentIndex(static_cast<RegularExp::Actions>(val));
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
