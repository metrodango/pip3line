#include "transformguibutton.h"
#include <QVBoxLayout>

TransformGuiButton::TransformGuiButton(TransformAbstract *transform, QWidget *parent) :
    QPushButton(parent),
    transform(transform),
    confDialog(nullptr)
{
    setIcon(QIcon(":/Images/icons/configure-5.png"));
    setMaximumWidth(25);
    setFlat(true);
    connect(transform, &TransformAbstract::destroyed, this, &TransformGuiButton::onTransformDeleted);
    connect(this, &TransformGuiButton::clicked, this, &TransformGuiButton::onGuiRequested);
}

TransformGuiButton::~TransformGuiButton()
{
    transform = nullptr;
    delete confDialog;
}

void TransformGuiButton::onGuiDeleted()
{
    // this should only be called when the dialog is deleted
    confDialog = nullptr;
}

void TransformGuiButton::onTransformDeleted()
{
    // this should never be called ... but who knows
    transform = nullptr;
}

void TransformGuiButton::onGuiRequested()
{
    if (transform != nullptr) {
        QWidget * guiConf = transform->getGui(nullptr);
        if (guiConf != nullptr) {
            connect(guiConf, &QWidget::destroyed, [=](QObject *) {onGuiDeleted();});
            QDialog * dialog = new(std::nothrow) QDialog();
            if (dialog == nullptr) {
                qFatal("Cannot allocate memory for QDialog [TransformGuiButton] X{");
            }
            dialog->setWindowTitle("Transform configuration");
            QVBoxLayout * layout = new(std::nothrow) QVBoxLayout();
            if (layout == nullptr) {
                qFatal("Cannot allocate memory for QVBoxLayout [TransformGuiButton] X{");
            }
            dialog->setLayout(layout);
            layout->addWidget(guiConf);
            QPushButton * closeButton = new(std::nothrow) QPushButton("Close");
            if (closeButton == nullptr) {
                qFatal("Cannot allocate memory for QPushButton [TransformGuiButton] X{");
            }

            closeButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
            closeButton->setFlat(true);
            layout->addWidget(closeButton, 0, Qt::AlignHCenter);
            connect(closeButton, &QPushButton::clicked, dialog, &QDialog::accept);
            connect(dialog, &QDialog::finished, dialog, &QDialog::deleteLater);
            dialog->open();
        }
    }
}
