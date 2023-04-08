/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "xorwidget.h"
#include <QDebug>
#include "ui_xorwidget.h"

XorWidget::XorWidget(Xor *ntransform, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::XorWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::XorWidget X{");
    }
    transform = ntransform;
    ui->setupUi(this);
    ui->typeComboBox->setFocusPolicy(Qt::StrongFocus);
    ui->typeComboBox->installEventFilter(this);
    connect(ui->keyPlainTextEdit, &QPlainTextEdit::textChanged,this, &XorWidget::onKeyChange);
    connect(ui->fromHexcheckBox, &QCheckBox::toggled, this, &XorWidget::onFromHexChange);
    //connect(ui->typeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &XorWidget::onTypeChange);
    connect(ui->typeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChange(int)));
    ui->keyPlainTextEdit->appendPlainText(transform->getKey());
    ui->fromHexcheckBox->setChecked(transform->isFromHex());
}

XorWidget::~XorWidget()
{
    delete ui;
}

void XorWidget::onFromHexChange(bool val)
{
    transform->setFromHex(val);
}

void XorWidget::onKeyChange()
{
    transform->setKey(ui->keyPlainTextEdit->toPlainText().toUtf8());
}

void XorWidget::onTypeChange(int index)
{
    switch (index) {
        case Xor::Basic:
            transform->setType(Xor::Basic);
            break;
        case Xor::PREVIOUSINPUT:
            transform->setType(Xor::PREVIOUSINPUT);
            break;
        case Xor::PREVIOUSOUTPUT:
            transform->setType(Xor::PREVIOUSOUTPUT);
            break;
        default:
            qCritical() << tr("Unknown index for Xor type T_T");
    }
}

bool XorWidget::eventFilter(QObject *o, QEvent *e)
{
    QEvent::Type etyp = e->type();
    // Filtering out wheel event for comboboxes
    if ( etyp == QEvent::Wheel && qobject_cast<QComboBox*>(o) )
    {
        e->ignore();
        return true;
    }
    return false;
}

void XorWidget::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
}
