#include "findandreplacewidget.h"
#include "ui_findandreplacewidget.h"
#include <QValidator>

const QString Hexvalidator::HEXCHAR("abcdefABCDEF0123456789");
Hexvalidator::Hexvalidator()
{

}

Hexvalidator::~Hexvalidator()
{

}

QValidator::State Hexvalidator::validate(QString &input, int &pos) const
{
    State ret = QValidator::Acceptable;
    if (input.size() % 2 != 0)
        ret = QValidator::Intermediate;
    for (int i = pos; i < input.size(); i++) {
        if (!HEXCHAR.contains(input.at(i))) {
            ret = QValidator::Invalid;
            break;
        }
    }
    return ret;
}

FindAndReplaceWidget::FindAndReplaceWidget(FindAndReplace *ntransform, QWidget *parent) :
    QWidget(parent),
    transform(ntransform)
{
    ui = new(std::nothrow) Ui::FindAndReplaceWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::FindAndReplaceWidget X{");
    }
    ui->setupUi(this);
    ui->searchLineEdit->setValidator(new Hexvalidator());
    ui->replaceLineEdit->setValidator(new Hexvalidator());
    ui->searchLineEdit->setText(transform->getSearchExpr());
    ui->replaceLineEdit->setText(transform->getReplaceExpr());
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, [=](const QString &value) {transform->setSearchExpr(value);});
    connect(ui->replaceLineEdit, &QLineEdit::textChanged, this, [=](const QString &value) {transform->setReplaceExpr(value);});
}

FindAndReplaceWidget::~FindAndReplaceWidget()
{
    delete ui;
}


