/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "pluginconfwidget.h"
#include "additionaluidialog.h"
#include "ui_pluginconfwidget.h"
#include <QDebug>
#include <QPushButton>

PluginConfWidget::PluginConfWidget(TransformFactoryPluginInterface *nplugin, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::PluginConfWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::PluginConfWidget X{");
    }
    plugin = nplugin;
    ui->setupUi(this);
    ui->pluginNamelabel->setText(plugin->pluginName());

    QWidget * gui = plugin->getConfGui(this);

    if (gui != nullptr) {
        ui->mainLayout->addWidget(gui);
    }

    QStringList typesList = plugin->getTypesList();

    QString types;
    for (int i = 0; i < typesList.size(); i++) {
        types.append(typesList.at(i)).append(": ");
        types.append(QString::number(plugin->getTransformList(typesList.at(i)).size()));
        types.append("\n");
    }

    ui->pluginTransformLabel->setText(types);
    QStringList packetHandlerList = plugin->getPacketHandlerList();
    if (!packetHandlerList.isEmpty()) {
        ui->additionalUIsGroupBox->setVisible(true);
        ui->additionalUIsGroupBox->setEnabled(true);
        for (int i = 0; i < packetHandlerList.size(); i++) {
            QString name = packetHandlerList.at(i);
            if (name.isEmpty()) {
                qCritical() << "[PluginConfWidget::PluginConfWidget] empty name for additional UI found, ignoring";
                continue;
            }
            QPushButton * uibutton = new(std::nothrow) QPushButton(name, ui->additionalUIsGroupBox);
            if (uibutton == nullptr) {
                qFatal("[PluginConfWidget::PluginConfWidget] Cannot allocate memory for QPushButton X{");
            }
            connect(uibutton, &QPushButton::clicked, this, &PluginConfWidget::onPacketHandlerClicked);
            ui->additionalUILayout->addWidget(uibutton);
        }
    } else {
        ui->additionalUIsGroupBox->setVisible(false);
        ui->additionalUIsGroupBox->setEnabled(false);
    }
}

PluginConfWidget::~PluginConfWidget()
{
    delete ui;
}

void PluginConfWidget::onPacketHandlerClicked()
{
    QPushButton * pb = dynamic_cast<QPushButton *>(sender());
    if (pb != nullptr) {
        QString name = pb->text();
        if (!name.isEmpty()) {
//            QWidget * additionalUI = plugin->getPacketHandler(name);
//            if (additionalUI != nullptr) {
//                AdditionalUIDialog * diag = new(std::nothrow) AdditionalUIDialog(additionalUI);
//                if (diag == nullptr) {
//                    qFatal("Cannot allocate memory for AdditionalUIDialog X{");
//                }
//                diag->show();
//                diag->raise();
//            } else {
//                qCritical() << "[PluginConfWidget::onAdditionalUIclicked] null widget returned for" << name << "T_T";
//            }
        } else {
            qCritical() << "[PluginConfWidget::onAdditionalUIclicked] empty name, ignoring T_T";
        }
    } else {
        qCritical() << "[PluginConfWidget::onAdditionalUIclicked] Cannot cast to QPushButton T_T";
    }
}
