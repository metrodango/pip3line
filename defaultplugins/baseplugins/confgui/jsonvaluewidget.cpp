#include "jsonvaluewidget.h"
#include "ui_jsonvaluewidget.h"

JsonValueWidget::JsonValueWidget(JsonValue *transform, QWidget *parent) :
    QWidget(parent),
    transform(transform)
{
    ui = new(std::nothrow) Ui::JsonValueWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::JsonValueWidget X{");
    }
    ui->setupUi(this);

    ui->valueNameLineEdit->setText(transform->getValueName());
    ui->compactCheckBox->setChecked(transform->getOutputJsonFormat() == QJsonDocument::Compact);
    connect(ui->compactCheckBox, &QCheckBox::toggled, this, [=] (bool val) { this->onCompactToggled(val);});
    connect(ui->valueNameLineEdit, &QLineEdit::textEdited, this, &JsonValueWidget::onNameChanged);
}

JsonValueWidget::~JsonValueWidget()
{
    delete ui;
}

void JsonValueWidget::onNameChanged(const QString &name)
{
    transform->setValueName(name);
}

void JsonValueWidget::onCompactToggled(bool checked)
{
    transform->setOutputJsonFormat(checked ? QJsonDocument::Compact : QJsonDocument::Indented);
}
