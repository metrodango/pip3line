#include "transformselectorwidget.h"
#include "ui_transformselectorwidget.h"
#include "guihelper.h"
#include <transformabstract.h>
#include <transformmgmt.h>

TransformSelectorWidget::TransformSelectorWidget(GuiHelper *guiHelper, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransformSelectorWidget),
    guiHelper(guiHelper),
    currentTransform(nullptr),
    confGui(nullptr)
{
    ui->setupUi(this);

    guiHelper->buildTransformComboBox(ui->transformComboBox);
    ui->directionGroupBox->setVisible(false);

    connect(ui->transformComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(onTransformSelected(QString)));
}

TransformSelectorWidget::~TransformSelectorWidget()
{
    delete ui;
}

void TransformSelectorWidget::onTransformSelected(QString name)
{
    if (confGui != nullptr)
        ui->mainLayout->removeWidget(confGui);
    delete currentTransform;

    TransformMgmt *transformFactory = TransformMgmt::getGlobalInstance();
    if (transformFactory != nullptr) {
        currentTransform = transformFactory->getTransform(name);

        if (currentTransform != nullptr) {
            connect(currentTransform, SIGNAL(destroyed()), this, SLOT(onTransformDelete()));
            if (currentTransform->isTwoWays()) {
                ui->inboundRadioButton->setText(currentTransform->inboundString());
                ui->outboundRadioButton->setText(currentTransform->outboundString());
                ui->directionGroupBox->setVisible(true);
                if (currentTransform->way() == TransformAbstract::INBOUND) {
                    ui->inboundRadioButton->setChecked(true);
                } else {
                    ui->outboundRadioButton->setChecked(true);
                }

            } else {
                ui->directionGroupBox->setVisible(false);
            }

            confGui = currentTransform->getGui(this);
            // never need to delete confgui, as it is taken care of by TransformAbstract upon destruction

            if (confGui != nullptr) {
                ui->mainLayout->addWidget(confGui);
            }
            emit newTransform(currentTransform);
        }
    } else {
        qCritical() << tr("[TransformSelectorWidget::onTransformSelected] No global object for TransformMgmt");
    }
}

void TransformSelectorWidget::onTransformDelete()
{
    confGui = nullptr;
}
