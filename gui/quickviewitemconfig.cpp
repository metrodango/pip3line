/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "quickviewitemconfig.h"
#include "ui_quickviewitemconfig.h"
#include "ui_transformdisplayconf.h"
#include <QDebug>

const QString QuickViewItemConfig::LOGID = "QuickViewItemConfig";
const QRegExp QuickViewItemConfig::LIMIT_NAME_REGEXP = QRegExp("^[a-zA-Z_][-a-zA-Z0-9_\\.]{1,100}$");

QuickViewItemConfig::QuickViewItemConfig(GuiHelper *nguiHelper, QWidget *parent) :
    QDialog(parent,Qt::WindowSystemMenuHint | Qt::WindowTitleHint),
    validator(LIMIT_NAME_REGEXP)
{
    ui = new(std::nothrow) Ui::QuickViewItemConfig();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::QuickViewItemConfig X{");
    }

    uiTransform = new(std::nothrow) Ui::TransformDisplayConf ();
    if (uiTransform == nullptr) {
        qFatal("Cannot allocate memory for Ui::TransformDisplayConf X{");
    }

    guiHelper = nguiHelper;
    ui->setupUi(this);
    uiTransform->setupUi(ui->confWidget);
    currentTransform = nullptr;
    confGui = nullptr;
    infoDialog = nullptr;
    readonlyVisible = false;
    wayBoxVisible = false;
    formatBoxVisible = false;
    outputTypeVisible = false;
    format = TEXTFORMAT;
    uiTransform->textRadioButton->setChecked(true);
    uiTransform->infoPushButton->setEnabled(false);
    uiTransform->readonlyCheckBox->setChecked(false);
    uiTransform->oneLineRadioButton->setChecked(true);

    transformFactory = guiHelper->getTransformFactory();

    guiHelper->buildTransformComboBox(uiTransform->transformComboBox);
    //connect(uiTransform->transformComboBox, qOverload<const QString &>(&QComboBox::currentIndexChanged), this, &QuickViewItemConfig::onTransformSelect);
    connect(uiTransform->transformComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onTransformSelect(QString)));
    connect(uiTransform->inboundRadioButton, &QRadioButton::toggled, this, &QuickViewItemConfig::onInboundWayChange);
    connect(uiTransform->infoPushButton, &QPushButton::clicked, this, &QuickViewItemConfig::onInfo);
    connect(uiTransform->textRadioButton, &QRadioButton::toggled, this, &QuickViewItemConfig::onTextFormatToggled);
}

QuickViewItemConfig::~QuickViewItemConfig()
{
    delete currentTransform;
    delete infoDialog;
    guiHelper = nullptr;
    delete uiTransform;
    delete ui;

}

void QuickViewItemConfig::closeEvent(QCloseEvent *event)
{
    reject();
    QDialog::closeEvent(event);
    deleteLater();
}

TransformAbstract *QuickViewItemConfig::getTransform()
{
    if (currentTransform != nullptr)
        return transformFactory->cloneTransform(currentTransform);
    else
        return nullptr;
}

void QuickViewItemConfig::setTransform(TransformAbstract *transform)
{
    if (transform != nullptr) {
        delete currentTransform;
        currentTransform = transformFactory->cloneTransform(transform);
        if (currentTransform != nullptr) {
            int index = uiTransform->transformComboBox->findText(currentTransform->name());
            if (index != -1) {
                uiTransform->transformComboBox->blockSignals(true);
                uiTransform->transformComboBox->setCurrentIndex(index);
                uiTransform->transformComboBox->blockSignals(false);
            } else {
                qWarning() << tr("[QuickViewItemConfig] Could not find the transform index T_T");
            }

            integrateTransform();
        }
    }
}

void QuickViewItemConfig::setName(const QString &name)
{
    uiTransform->nameLineEdit->setText(name);
}

QString QuickViewItemConfig::getName() const
{
    return uiTransform->nameLineEdit->text();
}

OutputFormat QuickViewItemConfig::getFormat() const
{
    return format;
}

void QuickViewItemConfig::setFormat(OutputFormat nformat)
{
    format = nformat;
    if (format == TEXTFORMAT)
        uiTransform->textRadioButton->setChecked(true);
    else
        uiTransform->hexaRadioButton->setChecked(true);
}

void QuickViewItemConfig::setOutputType(OutputType type)
{
    if (type == ONELINE) {
        uiTransform->oneLineRadioButton->setChecked(true);
    } else {
        uiTransform->blockRadioButton->setChecked(true);
    }
}

void QuickViewItemConfig::setReadonly(bool value)
{
    uiTransform->readonlyCheckBox->setChecked(value);
}

bool QuickViewItemConfig::getReadonly() const
{
    return uiTransform->readonlyCheckBox->isChecked();
}

OutputType QuickViewItemConfig::getOutputType()
{
    return (uiTransform->oneLineRadioButton->isChecked() ? ONELINE : MULTILINES);
}

void QuickViewItemConfig::setWayBoxVisible(bool val)
{
    wayBoxVisible = val;
    uiTransform->wayGroupBox->setVisible(wayBoxVisible);
}

void QuickViewItemConfig::setFormatVisible(bool val)
{
    formatBoxVisible = val;
    uiTransform->formatGroupBox->setVisible(formatBoxVisible);
}

void QuickViewItemConfig::setOutputTypeVisible(bool val)
{
    outputTypeVisible = val;
    uiTransform->typeGroupBox->setVisible(outputTypeVisible);
}

void QuickViewItemConfig::setReadonlyVisible(bool value)
{
    readonlyVisible = value;
    uiTransform->readonlyCheckBox->setVisible(readonlyVisible);
}

void QuickViewItemConfig::onTransformSelect(QString name)
{
    delete currentTransform;

    currentTransform = transformFactory->getTransform(name);

    if (currentTransform != nullptr)
        integrateTransform();

}

void QuickViewItemConfig::onInboundWayChange(bool checked)
{
    if (currentTransform != nullptr) {
        if (checked) {
            currentTransform->setWay(TransformAbstract::INBOUND);
        } else {
            currentTransform->setWay(TransformAbstract::OUTBOUND);
        }
    }
}

void QuickViewItemConfig::onTextFormatToggled(bool checked)
{
    if (checked)
        format = TEXTFORMAT;
    else
        format = HEXAFORMAT;
}

void QuickViewItemConfig::onInfo()
{
    if (currentTransform != nullptr) {
        if (infoDialog != nullptr && infoDialog->getTransform() != currentTransform){
            // if transform changed only
            delete infoDialog;
            infoDialog = nullptr;
        }

        infoDialog = new(std::nothrow) InfoDialog(guiHelper, currentTransform,this);
        if (infoDialog == nullptr) {
            qFatal("Cannot allocate memory for infoDialog (QuickViewItemConfig) X{");
        }
        else {
            infoDialog->setVisible(true);
        }
    }
}

void QuickViewItemConfig::integrateTransform()
{
    if (currentTransform != nullptr) {
        connect(currentTransform, &TransformAbstract::destroyed, this, &QuickViewItemConfig::onTransformDelete);
        if (currentTransform->isTwoWays()) {
            uiTransform->inboundRadioButton->setText(currentTransform->inboundString());
            uiTransform->outboundRadioButton->setText(currentTransform->outboundString());
            uiTransform->wayGroupBox->setVisible(wayBoxVisible);
            if (currentTransform->way() == TransformAbstract::INBOUND) {
                uiTransform->inboundRadioButton->setChecked(true);
            } else {
                uiTransform->outboundRadioButton->setChecked(true);
            }
        } else {
            uiTransform->wayGroupBox->setVisible(false);
        }
        uiTransform->formatGroupBox->setVisible(formatBoxVisible);
        uiTransform->typeGroupBox->setVisible(outputTypeVisible);

        if (confGui != nullptr)
            uiTransform->mainLayout->removeWidget(confGui);
        confGui = currentTransform->getGui(this);
        // never need to delete confgui, as it is taken care of by TransformAbstract upon destruction

        uiTransform->infoPushButton->setEnabled(true);
        if (uiTransform->nameLineEdit->text().isEmpty()) {
            uiTransform->nameLineEdit->setText(currentTransform->name());
        }
        if (confGui != nullptr) {
            uiTransform->mainLayout->addWidget(confGui);
        }
        this->adjustSize();
    }
}

void QuickViewItemConfig::onTransformDelete()
{
    currentTransform = nullptr;
    uiTransform->transformComboBox->blockSignals(true);
    uiTransform->transformComboBox->setCurrentIndex(0);
    uiTransform->transformComboBox->blockSignals(false);
    uiTransform->wayGroupBox->setVisible(false);
    uiTransform->formatGroupBox->setVisible(false);
}

bool QuickViewItemConfig::isLimitingNameCharacters() const
{
    return uiTransform->nameLineEdit->validator() != nullptr;
}

void QuickViewItemConfig::setLimitNameCharacters(bool enable)
{
    if (enable) {
        uiTransform->nameLineEdit->setValidator(&validator);
    } else {
        uiTransform->nameLineEdit->setValidator(nullptr);
    }
}
