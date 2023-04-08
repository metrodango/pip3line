/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "modulesmanagementwidget.h"
#include "ui_modulesmanagementwidget.h"
#include "deleteablelistitem.h"
#include "modulesmanagement.h"
#include <QFileDialog>
#include <QListWidgetItem>
#include <QDebug>

ModulesManagementWidget::ModulesManagementWidget(ModulesManagement *nmodulesMgmt, QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::ModulesManagementWidget();
    if (ui == nullptr) {
        qFatal("Cannot allocate memory for Ui::ModulesManagementWidget X{");
    }
    modulesMgmt = nmodulesMgmt;
    moduleTitle = modulesMgmt->getLangName();

    ui->setupUi(this);
    ui->infoLabel->setText(modulesMgmt->getInfos());
    connect(modulesMgmt, &ModulesManagement::modulesUpdated, this, &ModulesManagementWidget::loadModules);
    connect(modulesMgmt, &ModulesManagement::pathsUpdated, this, &ModulesManagementWidget::loadPaths);
    connect(ui->modulesListWidget, &QListWidget::itemClicked, this, &ModulesManagementWidget::onModuleSelected);
    connect(ui->loadPushButton, &QPushButton::clicked, this, &ModulesManagementWidget::onManualClicked);

    loadPaths();
    loadModules();
    qDebug() << this << "created";
}

ModulesManagementWidget::~ModulesManagementWidget()
{
    delete ui;
    qDebug() << this << "destroyed";
}

void ModulesManagementWidget::onManualClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose %1 module file").arg(moduleTitle),"", tr("%1 files (*%2)").arg(moduleTitle).arg(modulesMgmt->getExtension()));
    if (!fileName.isEmpty()) {
        modulesMgmt->addModule(fileName,ModulesManagement::PERSISTENT);
    }
}

void ModulesManagementWidget::loadModules()
{
    ui->modulesListWidget->clear();
    QStringList list = modulesMgmt->getModulesList();
    for (int i = 0; i < list.size(); i++) {
        DeleteableListItem *itemWid = new(std::nothrow) DeleteableListItem(list.at(i));
        if (itemWid == nullptr) {
            qFatal("Cannot allocate memory for DeleteableListItem X{");
        } else {
            if (modulesMgmt->getModuleType(list.at(i)) == ModulesManagement::AUTO) {
                itemWid->setEnableDelete(false);
            } else {
                itemWid->setEnableDelete(true);
            }

            connect(itemWid, &DeleteableListItem::itemDeleted, this, &ModulesManagementWidget::unload);
            QListWidgetItem *item = new(std::nothrow) QListWidgetItem();
            if (item == nullptr) {
                qFatal("Cannot allocate memory for QListWidgetItem X{");
            } else {
                ui->modulesListWidget->addItem(item);
                ui->modulesListWidget->setItemWidget(item, itemWid);
            }
        }
    }
    ui->modulesListWidget->setCurrentRow(0);

    ui->modulesListWidget->sortItems();
}

void ModulesManagementWidget::loadPaths()
{
    ui->pathListWidget->clear();
    ui->pathListWidget->addItems(modulesMgmt->getPathsList());
    ui->pathListWidget->sortItems();
}

void ModulesManagementWidget::unload(QString name)
{
    if (name.isEmpty())
        name = ui->modulesListWidget->currentItem()->text();

    if (!name.isEmpty())
        modulesMgmt->removeModule(name);

}

void ModulesManagementWidget::onModuleSelected(QListWidgetItem *item)
{
    QString name = (static_cast<DeleteableListItem *>(ui->modulesListWidget->itemWidget(item)))->getName();
}
