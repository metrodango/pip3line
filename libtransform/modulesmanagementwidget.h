/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MODULESMANAGEMENTWIDGET_H
#define MODULESMANAGEMENTWIDGET_H

#include <QWidget>
#include "libtransform_global.h"

namespace Ui {
class ModulesManagementWidget;
}

class QListWidgetItem;
class ModulesManagement;

class LIBTRANSFORMSHARED_EXPORT ModulesManagementWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit ModulesManagementWidget(ModulesManagement * modulesMgmt, QWidget *parent = 0);
        ~ModulesManagementWidget();

    private Q_SLOTS:
        void onManualClicked();
        void loadModules();
        void loadPaths();
        void unload(QString name = QString());
        void onModuleSelected(QListWidgetItem* item);
    private:
        Q_DISABLE_COPY(ModulesManagementWidget)
        QString moduleTitle;
        ModulesManagement * modulesMgmt;
        Ui::ModulesManagementWidget *ui;
};

#endif // MODULESMANAGEMENTWIDGET_H
