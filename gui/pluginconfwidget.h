/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PLUGINCONFWIDGET_H
#define PLUGINCONFWIDGET_H

#include <QWidget>
#include <transformfactoryplugininterface.h>
#include <QDialog>

namespace Ui {
class PluginConfWidget;
}

class PluginConfWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit PluginConfWidget(TransformFactoryPluginInterface * plugin, QWidget *parent = nullptr);
        ~PluginConfWidget();
    private slots:
        void onPacketHandlerClicked();
    private:
        Q_DISABLE_COPY(PluginConfWidget)
        Ui::PluginConfWidget *ui;
        TransformFactoryPluginInterface * plugin;
};

#endif // PLUGINCONFWIDGET_H
